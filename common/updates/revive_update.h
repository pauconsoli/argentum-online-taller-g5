#ifndef REVIVE_UPDATE_H
#define REVIVE_UPDATE_H

#include <cstdint>

#include "common/resurrect_status.h"
#include "common/updates/game_update.h"

class ReviveUpdate: public GameUpdate {
 private:
    uint32_t player_id;
    ResurrectStatus status;

 public:
    ReviveUpdate(uint32_t player_id, ResurrectStatus status):
        player_id(player_id), status(status) {}

    UpdateType get_type() const override {
        return UpdateType::REVIVE;
    }

    uint32_t get_target_player_id() const override {
        return player_id;
    }

    ResurrectStatus get_status() const {
        return status;
    }
};

#endif
