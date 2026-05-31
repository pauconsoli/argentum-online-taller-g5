#ifndef PLAYER_LEFT_UPDATE_H
#define PLAYER_LEFT_UPDATE_H

#include <cstdint>

#include "game_update.h"

class PlayerLeftUpdate: public GameUpdate {
 public:
    uint32_t player_id;

    explicit PlayerLeftUpdate(uint32_t player_id): player_id(player_id) {}

    UpdateType get_type() const override {
        return UpdateType::PLAYER_LEFT;
    }
};

#endif
