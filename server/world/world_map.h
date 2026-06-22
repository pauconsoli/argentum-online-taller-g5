#ifndef WORLD_MAP_H
#define WORLD_MAP_H

// Idea actual: hay dos grillas paralelas, una de celdas y otra de zonas, con las mismas dimensiones.
// Cada celda tiene un tipo de terreno y si es bloqueante o no, y cada zona (city/dungeon/normal)
// tiene sus propias características (ej si es segura o no, si se puede spawnear ahí, etc).

#include <memory>
#include <vector>

#include "cell.h"
#include "city.h"
#include "common/position.h"
#include "dungeon.h"
#include "zone.h"
#include "zone_type.h"

class WorldMap {
 private:
    std::vector<std::vector<Cell>>
        cells;  // usar un solo vectir, cuando indexo hacer suma, pero no está mal

    std::vector<std::vector<Zone*>> zones;
    std::vector<std::unique_ptr<Zone>> zones_storage;
    std::vector<City*> cities;
    std::vector<Dungeon*> dungeons;

    int width;
    int height;

 public:
    WorldMap(int width, int height);

    bool is_valid_position(const Position& pos) const;

    bool is_position_blocked(const Position& pos) const;

    const Cell& get_cell(const Position& pos) const;
    void set_cell(const Position& pos, const Cell& cell);

    Zone* get_zone(const Position& pos) const;
    void set_zone(const Position& pos, Zone* zone);
    void add_zone(std::unique_ptr<Zone> zone, int x, int y, int w, int h);
    void add_city(std::unique_ptr<City> city, int x, int y, int w, int h);
    void add_dungeon(std::unique_ptr<Dungeon> dungeon, int x, int y, int w, int h);

    City* get_closest_city(const Position& pos) const;

    const std::vector<City*>& get_cities() const;
    const std::vector<Dungeon*>& get_dungeons() const;

    bool is_safe(const Position& pos) const;

    ZoneType get_zone_type(const Position& pos) const;

    int get_width() const;
    int get_height() const;
};

#endif
