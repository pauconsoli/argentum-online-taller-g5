#ifndef LOOT_H
#define LOOT_H

#include <cstdint>
#include <vector>

#include "server/game/inventory.h"

struct Loot {
    uint64_t dropped_gold = 0;
    std::vector<InventorySlot> dropped_items;
};

#endif
