#ifndef CLIENT_MAP_H
#define CLIENT_MAP_H

#include <vector>

#include "common/world/terrain_type.h"

struct MapCell {
    TerrainType terrain;
    bool blocking;
};

class ClientMap {
 private:
    int width;
    int height;
    std::vector<MapCell> cells;  // row-major: cells[row * width + col]

 public:
    ClientMap(int width, int height, std::vector<MapCell> cells);

    const MapCell& at(int col, int row) const;
    int get_width() const;
    int get_height() const;
};

// TODO(chiaradelaurentis): reemplazar por construcción desde MapUpdate.
ClientMap build_sample_client_map();

#endif
