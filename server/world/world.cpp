#include "world.h"

#include <algorithm>
#include <cmath>
#include <queue>
#include <set>
#include <stdexcept>
#include <utility>

#include "server/game/game_config.h"
#include "server/game/game_formulas.h"
#include "server/game/items/magic_weapon.h"
#include "server/game/items/spell.h"
#include "server/game/items/weapon.h"

World::World(int width, int height):
    map(width, height), occupied(height, std::vector<bool>(width, false)) {}

World::World(WorldMap world_map):
    map(std::move(world_map)),
    occupied(map.get_height(), std::vector<bool>(map.get_width(), false)) {}


void World::add_player(std::unique_ptr<Player> player) {
    if (!player) {
        throw std::invalid_argument("World::add_player: intento de agregar un jugador nulo");
    }

    uint32_t id = player->get_id();
    if (player_exists(id)) {
        throw std::runtime_error("World::add_player: ya existe un jugador con ese ID");
    }

    Position pos = player->get_position();  // obtengo pos

    if (!map.is_valid_position(pos)) {
        throw std::out_of_range("World::add_player: la posición de spawn es inválida");
    }

    if (map.is_position_blocked(pos)) {
        throw std::runtime_error(
            "World::add_player: la posición de spawn está bloqueada por el terreno");
    }

    if (is_position_occupied(pos)) {
        throw std::runtime_error("World::add_player: la posición de spawn está ocupada");
    }

    occupied[pos.y][pos.x] = true;    // marco la posición como ocupada
    players[id] = std::move(player);  // agrego el player al map de players
}

void World::remove_player(uint32_t player_id) {
    auto it = players.find(player_id);
    if (it != players.end()) {
        Position pos = it->second->get_position();
        occupied[pos.y][pos.x] = false;  // marco la posición como libre
        players.erase(it);               // elimino el jugador
    }
}

Player* World::get_player(uint32_t player_id) {
    auto it = players.find(player_id);
    if (it != players.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<Player*> World::get_players() {
    std::vector<Player*> active_players;
    active_players.reserve(players.size());
    for (auto& [id, player] : players) {
        active_players.push_back(player.get());
    }
    return active_players;
}

bool World::player_exists(uint32_t player_id) const {
    return players.find(player_id) != players.end();
}

Character* World::get_character(uint32_t id) {
    if (Player* p = get_player(id)) {
        return p;
    }
    // TODO(Pau): buscar en mapa de NPCs cuando existan
    return nullptr;
}

// (0,0) es la esquina superior izquierda, x aumenta hacia la derecha e y hacia abajo
Position World::calculate_destination(const Position& current, Direction direction) const {
    Position dest = current;
    switch (direction) {
        case Direction::UP:
            dest.y -= 1;
            break;
        case Direction::DOWN:
            dest.y += 1;
            break;
        case Direction::LEFT:
            dest.x -= 1;
            break;
        case Direction::RIGHT:
            dest.x += 1;
            break;
    }
    return dest;
}

bool World::is_position_occupied(const Position& position) const {
    return occupied[position.y][position.x];
}

bool World::is_in_range_for_attack(const Player* attacker, const Character* target) const {
    bool is_ranged = false;
    Item* equipped_weapon = attacker->get_inventory().get_equipped_item(EquipmentSlot::WEAPON);
    if (equipped_weapon) {
        Weapon* weapon = static_cast<Weapon*>(
            equipped_weapon);  // mismo caso que en el cálculo de daño, el item equipado en ese slot
                               // tiene que ser un arma sí o sí
        is_ranged = weapon->is_ranged();
    }

    if (!is_ranged) {
        Position attacker_pos = attacker->get_position();
        Position target_pos = target->get_position();
        int dif_x = std::abs(attacker_pos.x - target_pos.x);
        int dif_y = std::abs(attacker_pos.y - target_pos.y);
        return dif_x <= 1 && dif_y <= 1;  // estoy adyacente
    }
    return true;  // a dist, rango ilimitado (visibilidad del objetivo)
}

// devuelve la posición de spawn para un nuevo jugador, que es la posición configurada en GameConfig
// pero si está ocupada, da la siguiente posición libre más cercana da la siguiente posición libre
// más cercana
Position World::get_spawn_position() const {
    const Position base_spawn = GameConfig::get_instance().get_spawn_position();

    for (int y = base_spawn.y; y < map.get_height(); ++y) {
        int x_start = (y == base_spawn.y) ? base_spawn.x : 0;  //
        for (int x = x_start; x < map.get_width(); ++x) {
            Position spawn{x, y};
            if (!map.is_position_blocked(spawn) && !is_position_occupied(spawn)) {
                return spawn;
            }
        }
    }

    for (int y = 0; y < base_spawn.y; ++y) {
        for (int x = 0; x < map.get_width(); ++x) {
            Position spawn{x, y};
            if (!map.is_position_blocked(spawn) && !is_position_occupied(spawn)) {
                return spawn;
            }
        }
    }

    throw std::runtime_error("World::get_spawn_position: no hay posiciones libres en el mapa");
}

const std::map<Position, GroundItem>& World::get_ground_items() const {
    return ground_items;
}

// no considera si esta ocupada porque el jugador se para sobre los items del suelo
Position World::find_closest_free_ground(const Position& start) const {
    std::queue<Position> q;
    std::set<Position> visited;

    q.push(start);
    visited.insert(start);

    while (!q.empty()) {
        Position curr = q.front();
        q.pop();
        if (map.is_valid_position(curr) && !map.is_position_blocked(curr)) {
            if (ground_items.find(curr) == ground_items.end()) {
                return curr;
            }
        }

        const Position neighbors[] = {
            {curr.x, curr.y - 1}, {curr.x, curr.y + 1}, {curr.x - 1, curr.y}, {curr.x + 1, curr.y}};

        for (const Position& neighbor : neighbors) {
            if (map.is_valid_position(neighbor) && !map.is_position_blocked(neighbor)) {
                if (visited.insert(neighbor).second) {
                    q.push(neighbor);
                }
            }
        }
    }
    throw std::runtime_error("World::find_closest_free_ground: no hay espacio libre en el suelo");
}

void World::drop_loot_in_world(const Position& center, Loot loot) {
    if (loot.dropped_gold > 0) {
        try {
            Position free_pos = find_closest_free_ground(center);
            ground_items[free_pos] = GroundItem{loot.dropped_gold, nullptr, 0};
        } catch (const std::exception&) {}
    }

    for (auto& slot : loot.dropped_items) {
        try {
            Position free_pos = find_closest_free_ground(center);
            ground_items[free_pos] = GroundItem{0, std::move(slot.item), slot.quantity};
        } catch (const std::exception&) {
            break;
        }
    }
}

void World::add_ground_item(const Position& pos, GroundItem item) {
    if (!map.is_valid_position(pos)) {
        throw std::runtime_error("World::add_ground_item: posición inválida");
    }

    Position free_pos = pos;
    if (map.is_position_blocked(pos) || ground_items.find(pos) != ground_items.end()) {
        free_pos = find_closest_free_ground(pos);
    }
    ground_items[free_pos] = std::move(item);
}

// TODO(Pau): refactor para que no tire excepciones, que devuelva un attack status o algo asi)
void World::validate_attack_conditions(const Player* attacker, const Character* target,
                                       bool is_healing) const {
    if (attacker->is_dead() || target->is_dead()) {
        throw std::runtime_error("World::attack: jugador u objetivo muertos, no se puede atacar");
    }

    if (!is_healing) {
        if (attacker->get_id() == target->get_id()) {
            throw std::runtime_error("World::attack: no puedes atacarte a ti mismo");
        }
    }

    if (!target->validate_attack_from(attacker->get_level())) {
        throw std::runtime_error(
            "World::attack: nivel insuficiente o diferencia de niveles no permitida");
    }

    if (!is_in_range_for_attack(attacker, target)) {
        throw std::runtime_error("World::attack: el objetivo está fuera de rango para el ataque");
    }
}


void World::consume_weapon_mana(Player* attacker) {
    Item* equipped_weapon = attacker->get_inventory().get_equipped_item(EquipmentSlot::WEAPON);
    if (equipped_weapon) {
        Weapon* weapon = static_cast<Weapon*>(equipped_weapon);
        int mana_cost = weapon->get_mana_cost();

        if (mana_cost > 0) {
            if (attacker->get_current_mana() < mana_cost) {
                throw std::runtime_error("World::attack: maná insuficiente para atacar");
            }
            attacker->consume_mana(mana_cost);
        }
    }
}


void World::handle_target_death(Player* attacker, Character* target) {
    int bonus_exp = GameFormulas::calculate_kill_experience_gain(*attacker, *target);
    attacker->add_experience(bonus_exp);

    Loot loot = target->drop_loot();
    drop_loot_in_world(target->get_position(), std::move(loot));
}


int World::handle_successful_attack(Player* attacker, Character* target, int damage) {
    int defense = target->get_defense();
    int real_damage = std::max(0, damage - defense);
    target->receive_damage(real_damage);

    int exp = GameFormulas::calculate_attack_experience_gain(*attacker, *target, real_damage);
    attacker->add_experience(exp);

    return real_damage;
}

// again esto no debería estar acá, pero por ahora solo quiero que funcione
MagicWeapon* World::get_healing_weapon(Player* attacker) const {
    Item* equipped_weapon = attacker->get_inventory().get_equipped_item(EquipmentSlot::WEAPON);
    if (MagicWeapon* magic_weapon = dynamic_cast<MagicWeapon*>(equipped_weapon)) {
        if (magic_weapon->get_spell().does_heal()) {
            return magic_weapon;
        }
    }
    return nullptr;
}

AttackResult World::handle_healing(uint32_t attacker_id, uint32_t target_id, Character* target,
                                   MagicWeapon* weapon) const {
    int min_heal = weapon->get_spell().get_min_heal();
    int max_heal = weapon->get_spell().get_max_heal();
    int heal_amount = GameFormulas::calculate_healing(min_heal, max_heal);

    target->heal(heal_amount);
    return AttackResult{attacker_id, target_id, 0, false, false, true, heal_amount};
}

// REFACTOR FUTURO.
// TODO(Pau): que se use Character* para poder usarlo para NPCs cuando existan
void World::move_player(uint32_t player_id, Direction direction) {
    auto it = players.find(player_id);
    if (it == players.end()) {
        throw std::runtime_error("World::move_player: jugador no encontrado");
    }

    Player* player = it->second.get();
    Position current = player->get_position();
    Position next = calculate_destination(current, direction);

    if (!map.is_valid_position(next)) {
        throw std::runtime_error("World::move_player: no podes avanzar en esa dirección.");
    }

    if (map.is_position_blocked(next)) {  // si la celda destino es bloqueante, no se mueve
        throw std::runtime_error("World::move_player: el paso está bloqueado");
    }

    if (is_position_occupied(next)) {
        throw std::runtime_error("World::move_player: hay alguien ocupando esa posición");
    }

    // si llegamos acá, la posición destino es válida, entonces se puede mover
    occupied[current.y][current.x] = false;
    player->set_position(next);
    occupied[next.y][next.x] = true;
}

// SOLO PARA TESTS
void World::set_cell(const Position& pos, const Cell& cell) {
    map.set_cell(pos, cell);
}


// TODO(Pau): clanes y zonas seguras
// esto hay que adaptarlo bien a NPCs, esta adaptado a la mitad
// el target puede ser NPC, pero en este momento no puede usarse para que un NPC ataqie

// otro refactor: que el hechizo heal no este incluido acá
AttackResult World::attack(uint32_t attacker_id, uint32_t target_id) {
    Player* attacker = get_player(attacker_id);
    Character* target = get_character(target_id);

    if (!attacker || !target) {
        throw std::runtime_error("World::attack: atacante u objetivo no existe");
    }

    MagicWeapon* healing_weapon = get_healing_weapon(attacker);
    bool is_healing = (healing_weapon != nullptr);

    validate_attack_conditions(attacker, target, is_healing);
    consume_weapon_mana(attacker);

    if (is_healing) {
        return handle_healing(attacker_id, target_id, target, healing_weapon);
    }

    int damage = GameFormulas::calculate_damage(*attacker);
    bool is_critical = GameFormulas::calculate_critical_attack();

    if (is_critical) {
        damage *= 2;  // extraer a config
    }

    bool evaded = !is_critical && GameFormulas::calculate_evasion(*target);

    int real_damage = 0;
    if (!evaded) {
        real_damage = handle_successful_attack(attacker, target, damage);
    }

    bool died = target->is_dead();
    if (died) {
        handle_target_death(attacker, target);
    }

    return AttackResult{attacker_id, target_id, real_damage, evaded, died, false, 0};
}

// para agarrar item TENGO QUE PARARME ARRIBA, uso la pos
void World::pick_up_item(uint32_t player_id) {
    Player* player = get_player(player_id);
    if (!player)
        throw std::runtime_error("World::pick_up_item: jugador no encontrado");
    if (player->is_dead())
        throw std::runtime_error("World::pick_up_item: un fantasma no puede agarrar items");

    Position pos = player->get_position();
    auto it = ground_items.find(pos);
    if (it == ground_items.end())
        throw std::runtime_error("World::pick_up_item: no hay item en esta posición");

    GroundItem& ground = it->second;

    if (ground.gold > 0) {
        player->add_gold(ground.gold);
        ground_items.erase(it);
        return;
    }

    if (!player->get_inventory().can_add_item(*ground.item)) {
        throw std::runtime_error("World::pick_up_item: inventario lleno");
    }

    player->get_inventory().add_item(std::move(ground.item), ground.quantity);

    ground_items.erase(it);
}


void World::drop_item(uint32_t player_id, int slot_index) {
    Player* player = get_player(player_id);
    if (!player)
        throw std::runtime_error("World::drop_item: jugador no encontrado");
    if (player->is_dead())
        throw std::runtime_error("World::drop_item: un fantasma no puede tirar items");

    InventorySlot slot = player->get_inventory().pop_slot(slot_index);
    if (!slot.item)
        throw std::runtime_error("World::drop_item: slot inválido o vacío");

    // si el item estaba equipado, no hace falta desquiparlo explícitamente
    // porque pop_slot ya lo saca del inventario completamente

    // no puedo dropear oro, solo items
    Position pos = find_closest_free_ground(player->get_position());
    ground_items[pos] = GroundItem{0, std::move(slot.item), slot.quantity};
}

void World::equip_item(uint32_t player_id, int slot_index) {
    Player* player = get_player(player_id);
    if (!player)
        throw std::runtime_error("World::equip_item: jugador no encontrado");
    if (player->is_dead())
        throw std::runtime_error("World::equip_item: un fantasma no puede equipar items");

    const auto& slots = player->get_inventory().get_slots();
    if (slot_index < 0 || slot_index >= static_cast<int>(slots.size())) {
        throw std::runtime_error("World::equip_item: índice de slot inválido");
    }
    if (!slots[slot_index].item) {
        throw std::runtime_error("World::equip_item: no hay ítem en ese slot");
    }

    player->equip(*slots[slot_index].item);
}

// esto podría devolver un vector de structs (Stats o similar) o algo indicando
// QUÉ cambió  y para QUE JUGADOR
void World::update(float tick_seconds) {
    for (auto& [id, player] : players) {
        if (player->is_dead())
            continue;

        // HP
        int hp_regen = GameFormulas::calculate_health_recovery(*player, tick_seconds);
        player->heal(hp_regen);

        // Mana (meditando o por tiempo)
        int mana_regen =
            player->is_meditating() ?
                GameFormulas::calculate_meditation_mana_recovery(*player, tick_seconds) :
                GameFormulas::calculate_time_mana_recovery(*player, tick_seconds);
        player->restore_mana(mana_regen);
    }

    // TODO(Pau): Update NPC states
    // TODO(Pau): Process world events (respawns, item drops, etc)
}
