#ifndef MOVED_UPDATE_H
#define MOVED_UPDATE_H

#include <cstdint>
#include "game_update.h"
#include "position.h"

class MovedUpdate : public GameUpdate {
private:
    uint32_t player_id;
    Position pos;

public:
    MovedUpdate(uint32_t player_id, Position pos)
        : player_id(player_id), pos(pos) {}

    UpdateType get_type() const override { return UpdateType::MOVED; }

    uint32_t get_player_id() const { return player_id; }
    Position get_pos() const { return pos; }
};

#endif