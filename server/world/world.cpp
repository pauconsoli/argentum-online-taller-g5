#include "world.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

#include "server/game/game_config.h"
#include "server/game/game_formulas.h"
#include "server/game/items/weapon.h"

// TODO(Pau): más verificaciones/excepciones/logging

World::World(int width, int height):
    map(width, height), occupied(height, std::vector<bool>(width, false)) {}

World::World(WorldMap world_map):
    map(std::move(world_map)),
    occupied(map.get_height(), std::vector<bool>(map.get_width(), false)) {}

void World::add_player(std::unique_ptr<Player> player) {
    Position pos = player->get_position();  // obtengo pos

    if (is_position_occupied(pos)) {
        return;  // acá iría una excepción
    }

    occupied[pos.y][pos.x] = true;                  // marco la posición como ocupada
    players[player->get_id()] = std::move(player);  // agrego el player al map de players
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
AttackResult World::attack(uint32_t attacker_id, uint32_t target_id) {
    Player* attacker = get_player(attacker_id);
    Character* target = get_character(target_id);

    if (!attacker || !target) {
        throw std::runtime_error("World::attack: atacante o objetivo no existe");
    }

    if (attacker->is_dead() || target->is_dead()) {
        throw std::runtime_error("World::attack: jugador u objetivo muertos, no se puede atacar");
    }

    if (!target->validate_attack_from(attacker->get_level())) {
        throw std::runtime_error(
            "World::attack: nivel insuficiente o diferencia de niveles no permitida");
    }

    if (!is_in_range_for_attack(attacker, target)) {
        throw std::runtime_error("World::attack: el objetivo está fuera de rango para el ataque");
    }

    Item* equipped_weapon = attacker->get_inventory().get_equipped_item(EquipmentSlot::WEAPON);
    if (equipped_weapon) {  // igual ver casteo
        Weapon* weapon =
            static_cast<Weapon*>(equipped_weapon);  // yo se que el item equipado en ese slot tiene
                                                    // que ser un arma sí o sí
        int mana_cost = weapon->get_mana_cost();

        if (mana_cost > 0) {
            if (attacker->get_current_mana() < mana_cost) {
                throw std::runtime_error("World::attack: maná insuficiente para atacar");
            }
            attacker->consume_mana(mana_cost);
        }
    }

    int damage = GameFormulas::calculate_damage(*attacker);
    bool is_critical = GameFormulas::calculate_critical_attack();

    if (is_critical) {
        damage *= 2;
    }

    bool evaded = !is_critical && GameFormulas::calculate_evasion(*target);

    int real_damage = 0;
    if (!evaded) {

        int defense = target->get_defense();
        real_damage = std::max(0, damage - defense);
        target->receive_damage(real_damage);

        int exp = GameFormulas::calculate_attack_experience_gain(*attacker, *target);
        attacker->add_experience(exp);
    }

    bool died = target->is_dead();
    if (died) {
        int bonus_exp = GameFormulas::calculate_kill_experience_gain(*attacker, *target);
        attacker->add_experience(bonus_exp);
        // TODO(Pau): drop de oro e items del atacado
    }

    return AttackResult{attacker_id, target_id, real_damage, evaded, died};
}


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
