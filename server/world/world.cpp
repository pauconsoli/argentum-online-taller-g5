#include "world.h"

#include <stdexcept>
#include <utility>

#include "server/game/game_config.h"
#include "server/game/game_formulas.h"

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

bool World::player_exists(uint32_t player_id) const {
    return players.find(player_id) != players.end();
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

bool World::move_player(uint32_t player_id, Direction direction) {
    auto it = players.find(player_id);
    if (it == players.end()) {
        return false;
    }

    Player* player = it->second.get();
    Position current = player->get_position();
    Position next = calculate_destination(current, direction);

    if (!map.is_valid_position(
            next)) {  // si la posición destino no es válida, no se mueve (ej fuera del mapa)
        return false;
    }

    if (map.is_position_blocked(next)) {  // si la celda destino es bloqueante, no se mueve
        return false;
    }

    if (is_position_occupied(next)) {  // si hay otro personaje en la posición destino, no se mueve
        return false;
    }

    // si llegamos acá, la posición destino es válida, entonces se puede mover
    occupied[current.y][current.x] = false;
    player->set_position(next);
    occupied[next.y][next.x] = true;

    return true;
}

// SOLO PARA TESTS
void World::set_cell(const Position& pos, const Cell& cell) {
    map.set_cell(pos, cell);
}


// refactor futuro: para que queden bien separadas las responsabilidades, esto no tendría que
// devolver Updates podría devolver un struct con los cambios que se hicieron (ej hp/mana
// recuperados, personajes que murieron, etc) y el Gameloop o el Match se encargaría de convertir
// eso en Updates para enviar a los clientes
std::vector<std::unique_ptr<GameUpdate>> World::update(float tick_seconds) {
    std::vector<std::unique_ptr<GameUpdate>> events;

    for (auto& [id, player] : players) {
        if (player->is_dead())
            continue;

        // HP
        int hp_regen = GameFormulas::calculate_health_recovery(*player, tick_seconds);
        player->heal(hp_regen);

        // Maná — meditando o por tiempo
        int mana_regen =
            player->is_meditating() ?
                GameFormulas::calculate_meditation_mana_recovery(*player, tick_seconds) :
                GameFormulas::calculate_time_mana_recovery(*player, tick_seconds);
        player->restore_mana(mana_regen);
    }

    // TODO(Pau): Update NPC states
    // TODO(Pau): Process world events (respawns, item drops, etc)

    return events;
}
