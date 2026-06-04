#ifndef GAME_UPDATE_H
#define GAME_UPDATE_H

#include <cstdint>
#include <memory>

enum class UpdateType : uint8_t {
    LOGIN_OK,
    MATCH_LIST,
    MATCH_CREATED,
    MATCH_JOINED,
    PLAYER_JOINED,
    PLAYER_LEFT,
    PLAYER_SPAWNED,
    ERROR,

    SNAPSHOT,
    SPAWNED,
    MOVED,
    STATS,
    DEATH,
    REVIVE,
    INVENTORY,
    CHAT_MSG,
};

class GameUpdate {
 public:
    virtual UpdateType get_type() const = 0;
    virtual ~GameUpdate() = default;
};

#endif
