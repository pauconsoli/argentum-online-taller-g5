#ifndef WORLD_MAP_H
#define WORLD_MAP_H

// WorldMap: representa el mapa del mundo Argentum, con sus celdas y zonas
// Idea actual: hay dos grillas paralelas, una de celdas y otra de zonas, con las mismas dimensiones. 
// Cada celda tiene un tipo de terreno y si es bloqueante o no, y cada zona (city/dungeon/normal) tiene 
// sus propias características (ej si es segura o no, si se puede spawnear ahí, etc).


#include <vector>

#include "cell.h"
#include "common/position.h"

class Zone;

class WorldMap {
private:
    std::vector<std::vector<Cell>> cells;

    std::vector<std::vector<Zone*>> zones;

    int width;
    int height;

public:
    WorldMap(int width, int height);

    bool is_valid_position(const Position& pos) const;

    bool is_blocking(const Position& pos) const;

    const Cell& get_cell(const Position& pos) const;
    void set_cell(const Position& pos, const Cell& cell);

    Zone* get_zone(const Position& pos) const;
    void set_zone(const Position& pos, Zone* zone);

    bool is_safe(const Position& pos) const;

    int get_width() const;
    int get_height() const;
};

#endif
