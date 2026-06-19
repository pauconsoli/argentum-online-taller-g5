#ifndef CLAN_REVIEW_RESULT_H
#define CLAN_REVIEW_RESULT_H

#include <string>
#include <vector>

#include "common/clan/clan_action_status.h"

struct ClanMemberInfo {
    uint32_t player_id;
    std::string nick;
    bool is_founder;
    bool is_online;
};

struct ClanReviewResult {
    ClanActionStatus status = ClanActionStatus::SUCCESS;
    std::string clan_name;
    std::vector<ClanMemberInfo> members;
    std::vector<ClanMemberInfo> pending;
};

#endif
