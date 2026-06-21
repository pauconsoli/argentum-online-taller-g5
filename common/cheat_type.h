#ifndef CHEAT_TYPE_H
#define CHEAT_TYPE_H

#include <cstdint>

enum class CheatType : uint8_t {
    HEAL_FULL = 0x01,
    RESTORE_MANA = 0x02,
    DIE = 0x03,
    LEVEL_UP = 0x04,
    GIVE_GOLD = 0x05,
};

struct CheatResult {
    bool success = false;
    bool player_died = false;
};

#endif
