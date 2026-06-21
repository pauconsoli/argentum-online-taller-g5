#ifndef CLAN_REVIEW_UPDATE_H
#define CLAN_REVIEW_UPDATE_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "common/clan/clan_review_result.h"
#include "common/updates/game_update.h"

class ClanReviewUpdate: public GameUpdate {
 private:
    uint32_t target_player_id;
    std::string clan_name;
    std::vector<ClanMemberInfo> members;
    std::vector<ClanMemberInfo> pending;

 public:
    ClanReviewUpdate(uint32_t target_player_id, std::string clan_name,
                     std::vector<ClanMemberInfo> members, std::vector<ClanMemberInfo> pending):
        target_player_id(target_player_id),
        clan_name(std::move(clan_name)),
        members(std::move(members)),
        pending(std::move(pending)) {}

    UpdateType get_type() const override {
        return UpdateType::CLAN_REVIEW;
    }

    uint32_t get_target_player_id() const override {
        return target_player_id;
    }

    const std::string& get_clan_name() const {
        return clan_name;
    }

    const std::vector<ClanMemberInfo>& get_members() const {
        return members;
    }

    const std::vector<ClanMemberInfo>& get_pending() const {
        return pending;
    }
};

#endif
