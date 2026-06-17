#ifndef SNAPSHOT_UPDATE_H
#define SNAPSHOT_UPDATE_H

#include <cstdint>
#include <string>
#include <utility>
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
    bool is_meditating;
    std::vector<std::string> equipment;
};

struct GroundItemSnapshot {
    int32_t x;
    int32_t y;
    bool is_gold;
    uint64_t quantity;
    std::string name;
};

struct NPCSnapshot {
    uint32_t npc_id;
    std::string name;
    int32_t x;
    int32_t y;
    int32_t hp;
    int32_t max_hp;
    bool is_hostile;
    uint32_t npc_type;
    uint8_t direction;
    bool is_moving;
};

class SnapshotUpdate: public GameUpdate {
 public:
    uint32_t tick;
    std::vector<PlayerSnapshot> players;
    std::vector<NPCSnapshot> npcs;
    std::vector<GroundItemSnapshot> ground_items;

    SnapshotUpdate(uint32_t tick, std::vector<PlayerSnapshot> players,
                   std::vector<NPCSnapshot> npcs, std::vector<GroundItemSnapshot> ground_items):
        tick(tick),
        players(std::move(players)),
        npcs(std::move(npcs)),
        ground_items(std::move(ground_items)) {}

    UpdateType get_type() const override {
        return UpdateType::SNAPSHOT;
    }
};

#endif
