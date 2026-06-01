#ifndef LOGIN_OK_UPDATE_H
#define LOGIN_OK_UPDATE_H

#include <cstdint>

#include "game_update.h"

class LoginOkUpdate: public GameUpdate {
 public:
    uint32_t player_id;

    explicit LoginOkUpdate(uint32_t player_id): player_id(player_id) {}

    UpdateType get_type() const override {
        return UpdateType::LOGIN_OK;
    }
};

#endif
