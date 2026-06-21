#ifndef PLAYER_LEFT_UPDATE_H
#define PLAYER_LEFT_UPDATE_H

#include <cstdint>
#include <string>
#include <utility>

#include "game_update.h"

class PlayerLeftUpdate: public GameUpdate {
 public:
    uint32_t player_id;
    std::string nick;
    uint32_t clan_id;

    PlayerLeftUpdate(uint32_t player_id, std::string nick, uint32_t clan_id):
        player_id(player_id), nick(std::move(nick)), clan_id(clan_id) {}

    UpdateType get_type() const override {
        return UpdateType::PLAYER_LEFT;
    }
};

#endif
