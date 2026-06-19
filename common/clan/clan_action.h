#ifndef CLAN_ACTION_H
#define CLAN_ACTION_H

#include <cstdint>

enum class ClanAction : uint8_t {
    FOUND = 0x01,
    JOIN_REQUEST = 0x02,
    REVIEW = 0x03,
    ACCEPT = 0x04,
    REJECT = 0x05,
    BAN = 0x06,
    KICK = 0x07,
    LEAVE = 0x08,
};

#endif
