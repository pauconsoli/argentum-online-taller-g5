#include "world_map.h"

#include <memory>
#include <utility>

#include "zone.h"

WorldMap::WorldMap(int width, int height):
    cells(height, std::vector<Cell>(width)),
    zones(height, std::vector<Zone*>(width, nullptr)),
    width(width),
    height(height) {}

bool WorldMap::is_valid_position(const Position& pos) const {
    return pos.x >= 0 && pos.x < width && pos.y >= 0 && pos.y < height;
}

bool WorldMap::is_position_blocked(const Position& pos) const {
    if (!is_valid_position(pos)) {
        return true;  // si la posición no es válida, es bloqueante (ej algo fuera del mapa)
    }

    return cells[pos.y][pos.x].is_blocking();
}

void WorldMap::set_zone(const Position& pos, Zone* zone) {
    if (!is_valid_position(pos))
        return;
    zones[pos.y][pos.x] = zone;
}

Zone* WorldMap::get_zone(const Position& pos) const {
    if (!is_valid_position(pos)) {
        return nullptr;  // si la posición no es válida, no hay zona (ej algo fuera del mapa)
    }

    return zones[pos.y][pos.x];
}

void WorldMap::add_zone(std::unique_ptr<Zone> zone, int x, int y, int w, int h) {
    Zone* ptr = zone.get();
    zones_storage.push_back(std::move(zone));

    for (int dy = 0; dy < h; dy++) {
        for (int dx = 0; dx < w; dx++) {
            set_zone({x + dx, y + dy}, ptr);
        }
    }
}

void WorldMap::add_city(std::unique_ptr<City> city, int x, int y, int w, int h) {
    add_zone(std::move(city), x, y, w, h);
    cities.push_back(static_cast<City*>(zones_storage.back().get()));
}

void WorldMap::add_dungeon(std::unique_ptr<Dungeon> dungeon, int x, int y, int w, int h) {
    add_zone(std::move(dungeon), x, y, w, h);
    dungeons.push_back(static_cast<Dungeon*>(zones_storage.back().get()));
}

City* WorldMap::get_closest_city(const Position& pos) const {
    City* closest = nullptr;
    int min_dist = -1;

    for (City* city : cities) {
        Position priest_pos = city->get_priest_position();
        // distancia al cuadrado para no tener que calcular la raíz
        int dist_sq = (priest_pos.x - pos.x) * (priest_pos.x - pos.x) +
                      (priest_pos.y - pos.y) * (priest_pos.y - pos.y);
        if (closest == nullptr || dist_sq < min_dist) {
            closest = city;
            min_dist = dist_sq;
        }
    }

    return closest;
}

const std::vector<City*>& WorldMap::get_cities() const {
    return cities;
}

const std::vector<Dungeon*>& WorldMap::get_dungeons() const {
    return dungeons;
}

void WorldMap::set_cell(const Position& pos, const Cell& cell) {
    if (!is_valid_position(pos))
        return;
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
    const Zone* zone = get_zone(pos);
    return zone != nullptr && zone->is_safe();
}

ZoneType WorldMap::get_zone_type(const Position& pos) const {
    const Zone* zone = get_zone(pos);
    if (!zone)
        return ZoneType::ALL;
    return zone->get_type();
}

int WorldMap::get_width() const {
    return width;
}

int WorldMap::get_height() const {
    return height;
}
