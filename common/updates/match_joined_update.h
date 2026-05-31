#ifndef MATCH_JOINED_UPDATE_H
#define MATCH_JOINED_UPDATE_H

#include <cstdint>

#include "game_update.h"

class MatchJoinedUpdate: public GameUpdate {
 public:
    uint32_t match_id;
    uint32_t your_player_id;

    MatchJoinedUpdate(uint32_t match_id, uint32_t your_player_id):
            match_id(match_id), your_player_id(your_player_id) {}

    UpdateType get_type() const override {
        return UpdateType::MATCH_JOINED;
    }
};

#endif
