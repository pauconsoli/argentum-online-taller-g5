#ifndef WORLD_ITEM_LOADER_H
#define WORLD_ITEM_LOADER_H

#include <string>

#include "server/world/world.h"

class WorldItemLoader {
 public:
    static void load_ground_items(World& world, const std::string& map_config_path);
};

#endif
