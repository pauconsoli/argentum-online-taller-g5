#ifndef SNAPSHOT_UPDATE_H
#define SNAPSHOT_UPDATE_H

#include <cstdint>
#include <string>
#include <vector>

#include "game_update.h"

struct PlayerSnapshot {
    uint32_t player_id;
    std::string nick;
    uint8_t race;
    uint8_t klass;
    int32_t x;
    int32_t y;
    int32_t hp;
    int32_t max_hp;
    int32_t mp;
    int32_t max_mp;
    uint64_t xp;
    uint64_t gold;
    uint16_t level;
    bool is_ghost;
};

class SnapshotUpdate: public GameUpdate {
 public:
    uint32_t tick;
    std::vector<PlayerSnapshot> players;

    SnapshotUpdate(uint32_t tick, std::vector<PlayerSnapshot> players):
            tick(tick), players(std::move(players)) {}

    UpdateType get_type() const override {
        return UpdateType::SNAPSHOT;
    }
};

#endif
