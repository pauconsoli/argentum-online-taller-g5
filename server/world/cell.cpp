#include "cell.h"

Cell::Cell(
        TerrainType terrain_type,
        bool blocking):
        terrain_type(terrain_type),
        blocking(blocking) {}

TerrainType Cell::get_terrain_type() const {
    return terrain_type;
}

bool Cell::is_blocking() const {
    return blocking;
}
