#ifndef MOVED_UPDATE_H
#define MOVED_UPDATE_H

#include <cstdint>
#include "game_update.h"

class MovedUpdate : public GameUpdate {
private:
    uint16_t player_id;
    uint16_t x;
    uint16_t y;

public:
    MovedUpdate(uint16_t player_id, uint16_t x, uint16_t y)
        : player_id(player_id), x(x), y(y) {}

    UpdateType get_type() const override { return UpdateType::MOVED; }

    uint16_t get_player_id() const { return player_id; }
    uint16_t get_x() const { return x; }
    uint16_t get_y() const { return y; }
};

#endif