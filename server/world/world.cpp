#include "world.h"

#include <stdexcept>
#include <utility>

#include "common/updates/move_update.h"

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

std::unique_ptr<GameUpdate> World::move_player(uint32_t player_id, Direction direction) {
    auto it = players.find(player_id);
    if (it == players.end()) {
        return nullptr;
    }

    Player* player = it->second.get();
    Position current = player->get_position();
    Position next = calculate_destination(current, direction);

    if (!map.is_valid_position(
            next)) {     // si la posición destino no es válida, no se mueve (ej fuera del mapa)
        return nullptr;  // update: no cambio nada
    }

    if (map.is_position_blocked(next)) {  // si la celda destino es bloqueante, no se mueve
        return nullptr;                   // update: no cambio nada
    }

    if (is_position_occupied(next)) {  // si hay otro personaje en la posición destino, no se mueve
        return nullptr;                // update: no cambio nada
    }

    // si llegamos acá, la posición destino es válida, entonces se puede mover
    occupied[current.y][current.x] = false;
    player->set_position(next);
    occupied[next.y][next.x] = true;

    // update: devuelvo un update con la nueva posición del jugador
    return std::make_unique<MovedUpdate>(player_id, next);
}

// SOLO PARA TESTS
void World::set_cell(const Position& pos, const Cell& cell) {
    map.set_cell(pos, cell);
}

std::vector<std::unique_ptr<GameUpdate>> World::update() {
    std::vector<std::unique_ptr<GameUpdate>> events;

    // TODO(Pau): Update player health/mana regeneration
    // TODO(Pau): Update NPC states
    // TODO(Pau): Process world events (respawns, item drops, etc)

    return events;
}
