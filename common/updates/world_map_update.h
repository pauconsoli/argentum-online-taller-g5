#ifndef WORLD_MAP_UPDATE_H
#define WORLD_MAP_UPDATE_H

#include <cstdint>
#include <utility>
#include <vector>

#include "game_update.h"

struct MapCellData {
    uint8_t terrain_type;
    bool blocking;
};

class WorldMapUpdate: public GameUpdate {
 public:
    uint16_t width;
    uint16_t height;
    std::vector<MapCellData> cells;

    WorldMapUpdate(uint16_t width, uint16_t height, std::vector<MapCellData> cells):
            width(width), height(height), cells(std::move(cells)) {}

    UpdateType get_type() const override {
        return UpdateType::WORLD_MAP;
    }
};

#endif
