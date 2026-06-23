#ifndef CLAN_ACTION_STATUS_H
#define CLAN_ACTION_STATUS_H

#include <cstdint>
#include <string>

enum class ClanActionStatus : uint8_t {
    SUCCESS = 0,
    LEVEL_TOO_LOW,
    ALREADY_IN_CLAN,
    NAME_TAKEN,
    NAME_EMPTY,
    CLAN_NOT_FOUND,
    NOT_FOUNDER,
    NOT_IN_CLAN,
    NOT_PENDING,
    CLAN_FULL,
    TARGET_OFFLINE,
    TARGET_IS_SELF,
    FOUNDER_CANNOT_LEAVE,
    PLAYER_BANNED,
    INTERNAL_ERROR,
};

#endif
