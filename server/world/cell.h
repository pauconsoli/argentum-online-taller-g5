#ifndef CELL_H
#define CELL_H

#include "common/world/terrain_type.h"

class Cell {
 private:
    TerrainType terrain_type;

    bool blocking;

 public:
    explicit Cell(TerrainType terrain_type = TerrainType::GRASS, bool blocking = false);

    TerrainType get_terrain_type() const;

    bool is_blocking() const;
};

#endif
