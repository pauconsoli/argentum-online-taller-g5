#ifndef WORLD_MAP_LOADER_H
#define WORLD_MAP_LOADER_H

#include <string>

#include "server/world/world_map.h"

// cargar un WorldMap desde del toml de config
class WorldMapLoader {
 public:
    static WorldMap load(const std::string& config_path);
};

#endif
