#ifndef MATCH_JOINED_UPDATE_H
#define MATCH_JOINED_UPDATE_H

#include <cstdint>

#include "game_update.h"

class MatchJoinedUpdate: public GameUpdate {
 public:
    uint32_t match_id;
    uint32_t your_player_id;
    bool was_restored;
    uint8_t restored_race;
    uint8_t restored_klass;

    MatchJoinedUpdate(uint32_t match_id, uint32_t your_player_id,
                      bool was_restored = false,
                      uint8_t restored_race = 0, uint8_t restored_klass = 0):
        match_id(match_id),
        your_player_id(your_player_id),
        was_restored(was_restored),
        restored_race(restored_race),
        restored_klass(restored_klass) {}

    UpdateType get_type() const override {
        return UpdateType::MATCH_JOINED;
    }
};

#endif
