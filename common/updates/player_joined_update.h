#ifndef PLAYER_JOINED_UPDATE_H
#define PLAYER_JOINED_UPDATE_H

#include <cstdint>
#include <string>
#include <utility>

#include "common/position.h"
#include "game_update.h"

class PlayerJoinedUpdate: public GameUpdate {
 public:
    uint32_t player_id;
    std::string nick;
    uint8_t race;
    uint8_t klass;
    Position pos;

    PlayerJoinedUpdate(uint32_t player_id, std::string nick, uint8_t race, uint8_t klass,
                       Position pos):
        player_id(player_id), nick(std::move(nick)), race(race), klass(klass), pos(pos) {}

    UpdateType get_type() const override {
        return UpdateType::PLAYER_JOINED;
    }
};

#endif
