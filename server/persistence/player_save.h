#ifndef PLAYER_SAVE_H
#define PLAYER_SAVE_H

#include <cstdint>
#include <string>
#include <vector>

struct InventorySlotSave {
    std::string item_name; 
    uint32_t quantity = 0;
    bool is_equipped = false;
};

struct PlayerSave {
    std::string nick;
    std::string match_name;

    uint8_t race = 0;
    uint8_t klass = 0;

    uint16_t level = 1;
    uint64_t xp = 0;

    int32_t hp = 0;
    int32_t max_hp = 0;
    int32_t mp = 0;
    int32_t max_mp = 0;

    uint64_t gold = 0;

    int32_t pos_x = 0;
    int32_t pos_y = 0;

    std::vector<InventorySlotSave> inventory;
};

struct MatchSave {
    std::string name;       
    uint8_t max_players = 4;
};

struct WorldSnapshot {
    uint32_t version = 1;       
    int64_t saved_at_unix = 0;  
    std::vector<PlayerSave> players;
    std::vector<MatchSave> matches;
};

#endif
