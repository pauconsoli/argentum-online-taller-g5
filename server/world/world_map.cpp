#include "world_map.h"
#include "zone.h"

// TODO: implementar excepciones para casos de posición inválida, etc. (ej algo fuera del mapa) 
// y usarlas en vez de retornar nullptr, true o return vacío. Más fácil detectaar errores

WorldMap::WorldMap(int width, int height):
        cells(height, std::vector<Cell>(width)),
        zones(height, std::vector<Zone*>(width, nullptr)),
        width(width),
        height(height) {}

bool WorldMap::is_valid_position(const Position& pos) const {
    return pos.x >= 0 && pos.x < width &&
           pos.y >= 0 && pos.y < height;
}

bool WorldMap::is_position_blocked(const Position& pos) const {
    if (!is_valid_position(pos)) {
        return true; //si la posición no es válida, es bloqueante (ej algo fuera del mapa)
    }

    return cells[pos.y][pos.x].is_blocking();
}

void WorldMap::set_zone(const Position& pos, Zone* zone) {
    if (!is_valid_position(pos)) return;
    zones[pos.y][pos.x] = zone;
}

Zone* WorldMap::get_zone(const Position& pos) const {

    if (!is_valid_position(pos)) {
        return nullptr; //si la posición no es válida, no hay zona (ej algo fuera del mapa)
    } 

    return zones[pos.y][pos.x];
}

void WorldMap::set_cell(const Position& pos, const Cell& cell) {
    if (!is_valid_position(pos)) return;
    cells[pos.y][pos.x] = cell;
}

const Cell& WorldMap::get_cell(const Position& pos) const {
    if (!is_valid_position(pos)) {
        static const Cell empty_cell;
        return empty_cell;
    }
    return cells[pos.y][pos.x];
}

bool WorldMap::is_safe(const Position& pos) const {
    Zone* zone = get_zone(pos);
    return zone != nullptr && zone->is_safe();
}

int WorldMap::get_width() const {
    return width;
}

int WorldMap::get_height() const {
    return height;
}
