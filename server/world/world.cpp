#include "world.h"

#include <algorithm>
#include <cmath>
#include <queue>
#include <set>
#include <stdexcept>
#include <utility>

#include "server/game/game_config.h"
#include "server/game/game_formulas.h"
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
    Weapon* weapon = attacker->get_equipped_weapon();
    if (weapon) {
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
std::optional<Position> World::find_closest_free_ground(const Position& start) const {
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
    return std::nullopt;
}

std::optional<Position> World::find_closest_unoccupied_position(const Position& start) const {
    std::queue<Position> q;
    std::set<Position> visited;

    q.push(start);
    visited.insert(start);

    while (!q.empty()) {
        Position curr = q.front();
        q.pop();
        if (map.is_valid_position(curr) && !map.is_position_blocked(curr)) {
            if (!is_position_occupied(curr)) {
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
    return std::nullopt;
}

void World::drop_loot_in_world(const Position& center, Loot loot) {
    if (loot.dropped_gold > 0) {
        if (auto free_pos = find_closest_free_ground(center)) {
            ground_items[*free_pos] = GroundItem{loot.dropped_gold, nullptr, 0};
        }
    }

    for (auto& slot : loot.dropped_items) {
        if (auto free_pos = find_closest_free_ground(center)) {
            ground_items[*free_pos] = GroundItem{0, std::move(slot.item), slot.quantity};
        } else {
            break;
        }
    }
}

void World::add_ground_item(const Position& pos, GroundItem item) {
    if (!map.is_valid_position(pos)) {
        return;
    }

    Position free_pos = pos;
    if (map.is_position_blocked(pos) || ground_items.find(pos) != ground_items.end()) {
        if (auto maybe_pos = find_closest_free_ground(pos)) {
            free_pos = *maybe_pos;
        } else {
            return;
        }
    }
    ground_items[free_pos] = std::move(item);
}

AttackStatus World::validate_attack_conditions(const Player* attacker, const Character* target,
                                               bool is_healing) const {
    if (attacker->is_dead() || target->is_dead()) {
        return AttackStatus::DEAD;
    }

    if (!is_healing) {
        if (attacker->get_id() == target->get_id()) {
            return AttackStatus::INVALID_TARGET;
        }

        if (!target->validate_attack_from(attacker->get_level())) {
            return AttackStatus::INVALID_TARGET;
        }

        // En zonas seguras (ciudades y pueblos) no se puede atacar ni ser atacado
        if (map.is_safe(attacker->get_position()) || map.is_safe(target->get_position())) {
            return AttackStatus::INVALID_TARGET;
        }
    }

    if (!is_in_range_for_attack(attacker, target)) {
        return AttackStatus::OUT_OF_RANGE;
    }

    return AttackStatus::SUCCESS;
}


AttackStatus World::consume_weapon_mana(Player* attacker) {
    Weapon* weapon = attacker->get_equipped_weapon();
    if (weapon) {
        int mana_cost = weapon->get_mana_cost();

        if (mana_cost > 0) {
            if (attacker->get_current_mana() < mana_cost) {
                return AttackStatus::NO_MANA;
            }
            attacker->consume_mana(mana_cost);
        }
    }
    return AttackStatus::SUCCESS;
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

// REFACTOR FUTURO.
// TODO(Pau): que se use Character* para poder usarlo para NPCs cuando existan
bool World::move_player(uint32_t player_id, Direction direction) {
    auto it = players.find(player_id);
    if (it == players.end()) {
        return false;
    }

    Player* player = it->second.get();
    Position current = player->get_position();
    Position next = calculate_destination(current, direction);

    if (!map.is_valid_position(next)) {
        return false;
    }

    if (map.is_position_blocked(next)) {  // si la celda destino es bloqueante, no se mueve
        return false;
    }

    if (is_position_occupied(next)) {
        return false;
    }

    // si llegamos acá, la posición destino es válida, entonces se puede mover
    occupied[current.y][current.x] = false;
    player->set_position(next);
    occupied[next.y][next.x] = true;
    return true;
}

bool World::teleport_player(uint32_t player_id, const Position& dest) {
    Player* player = get_player(player_id);
    if (!player)
        return false;

    auto maybe_dest = find_closest_unoccupied_position(dest);
    if (!maybe_dest)
        return false;

    Position final_dest = *maybe_dest;
    Position current = player->get_position();
    occupied[current.y][current.x] = false;
    player->set_position(final_dest);
    occupied[final_dest.y][final_dest.x] = true;
    return true;
}

bool World::start_resurrection(uint32_t player_id) {
    Player* player = get_player(player_id);
    if (!player || !player->is_dead()) {
        return false;
    }

    const City* closest_city = map.get_closest_city(player->get_position());
    if (!closest_city) {
        return false;  // no hay ciudades
    }

    // TODO(Pau): La operación no debería ser inmediata según el enunciado.
    // El fantasma debería quedar inmovilizado un tiempo proporcional a la distancia
    // entre él y el sanador antes de trasladarse y resucitar

    Position priest_pos = closest_city->get_priest_position();

    if (teleport_player(player_id, priest_pos)) {
        // TODO(Pau): Acá se debería llamar a la lógica en Player para restaurar salud, etc.
        return true;
    }

    return false;
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
        return AttackResult{attacker_id, target_id, 0, false,
                            false,       false,     0, AttackStatus::INVALID_TARGET};
    }

    Weapon* weapon = attacker->get_equipped_weapon();
    bool is_healing = weapon ? weapon->is_healing() : false;

    AttackStatus status = validate_attack_conditions(attacker, target, is_healing);
    if (status != AttackStatus::SUCCESS) {
        return AttackResult{attacker_id, target_id, 0, false, false, false, 0, status};
    }
    status = consume_weapon_mana(attacker);
    if (status != AttackStatus::SUCCESS) {
        return AttackResult{attacker_id, target_id, 0, false, false, false, 0, status};
    }

    WeaponEffect effect;
    if (weapon) {
        effect = weapon->apply_effect(*attacker);
    } else {
        effect.damage = GameFormulas::calculate_damage(*attacker);  // daño base sin arma
    }

    if (is_healing) {
        target->heal(effect.healing);
        return AttackResult{attacker_id,          target_id, 0, false, false, true, effect.healing,
                            AttackStatus::SUCCESS};
    }

    bool is_critical = GameFormulas::calculate_critical_attack();

    if (is_critical) {
        effect.damage *= 2;  // extraer a config
    }

    bool evaded = !is_critical && GameFormulas::calculate_evasion(*target);

    int real_damage = 0;
    if (!evaded) {
        real_damage = handle_successful_attack(attacker, target, effect.damage);
    }

    bool died = target->is_dead();
    if (died) {
        handle_target_death(attacker, target);
    }

    return AttackResult{attacker_id, target_id, real_damage, evaded,
                        died,        false,     0,           AttackStatus::SUCCESS};
}

// para agarrar item TENGO QUE PARARME ARRIBA, uso la pos
bool World::pick_up_item(uint32_t player_id) {
    Player* player = get_player(player_id);
    if (!player)
        return false;
    if (player->is_dead())
        return false;

    Position pos = player->get_position();
    auto it = ground_items.find(pos);
    if (it == ground_items.end())
        return false;

    GroundItem& ground = it->second;

    if (ground.gold > 0) {
        player->add_gold(ground.gold);
        ground_items.erase(it);
        return true;
    }

    if (!player->get_inventory().can_add_item(*ground.item)) {
        return false;
    }

    player->get_inventory().add_item(std::move(ground.item), ground.quantity);

    ground_items.erase(it);
    return true;
}


bool World::drop_item(uint32_t player_id, int slot_index) {
    Player* player = get_player(player_id);
    if (!player)
        return false;
    if (player->is_dead())
        return false;

    auto maybe_pos = find_closest_free_ground(player->get_position());
    if (!maybe_pos)
        return false;

    InventorySlot slot = player->get_inventory().pop_slot(slot_index);
    if (!slot.item)
        return false;

    ground_items[*maybe_pos] = GroundItem{0, std::move(slot.item), slot.quantity};
    return true;
}

bool World::equip_item(uint32_t player_id, int slot_index) {
    Player* player = get_player(player_id);
    if (!player)
        return false;
    if (player->is_dead())
        return false;

    const auto& inv_slots = player->get_inventory().get_slots();
    int num_slots = inv_slots.size();
    if (slot_index < 0 || slot_index >= num_slots) {
        return false;
    }
    if (!inv_slots[slot_index].item) {
        return false;
    }

    player->equip(*inv_slots[slot_index].item);
    return true;
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
