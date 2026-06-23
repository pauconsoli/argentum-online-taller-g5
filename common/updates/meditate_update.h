#ifndef MEDITATE_UPDATE_H
#define MEDITATE_UPDATE_H

#include <cstdint>

#include "common/meditate_status.h"
#include "common/updates/game_update.h"

class MeditateUpdate: public GameUpdate {
 private:
    uint32_t player_id;
    MeditateStatus status;

 public:
    MeditateUpdate(uint32_t player_id, MeditateStatus status):
        player_id(player_id), status(status) {}

    UpdateType get_type() const override {
        return UpdateType::MEDITATE;
    }

    uint32_t get_target_player_id() const override {
        return player_id;
    }

    MeditateStatus get_status() const {
        return status;
    }
};

#endif
