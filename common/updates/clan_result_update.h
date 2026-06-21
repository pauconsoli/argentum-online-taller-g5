#ifndef CLAN_RESULT_UPDATE_H
#define CLAN_RESULT_UPDATE_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "common/clan/clan_action.h"
#include "common/clan/clan_result.h"
#include "common/updates/game_update.h"

class ClanResultUpdate: public GameUpdate {
 private:
    uint32_t target_player_id;
    ClanAction action;
    ClanResult result;
    std::string clan_name;
    std::string other_nick;  // según el action: target del comando, o quien lo originó

 public:
    ClanResultUpdate(uint32_t target_player_id, ClanAction action, ClanResult result,
                     std::string clan_name = "", std::string other_nick = ""):
        target_player_id(target_player_id),
        action(action),
        result(result),
        clan_name(std::move(clan_name)),
        other_nick(std::move(other_nick)) {}

    UpdateType get_type() const override {
        return UpdateType::CLAN_RESULT;
    }

    uint32_t get_target_player_id() const override {
        return target_player_id;
    }

    ClanAction get_action() const {
        return action;
    }

    ClanResult get_result() const {
        return result;
    }

    const std::string& get_clan_name() const {
        return clan_name;
    }

    const std::string& get_other_nick() const {
        return other_nick;
    }
};

#endif
