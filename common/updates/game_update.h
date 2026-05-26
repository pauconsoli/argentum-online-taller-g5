#ifndef GAME_UPDATE_H
#define GAME_UPDATE_H

#include <cstdint>

enum class UpdateType : uint8_t {
    MOVED = 0x01,
    STATS = 0x02,
    INVENTORY = 0x03,
    MESSAGE = 0x04,
    DEATH = 0x05,
    REVIVE = 0x06,
};

class GameUpdate {
 public:
    virtual UpdateType get_type() const = 0;
    virtual ~GameUpdate() = default;
};

#endif
