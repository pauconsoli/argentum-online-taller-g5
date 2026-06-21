#include "common/commands/clan_command.h"

#include <memory>
#include <vector>

#include "common/clan/clan_action_status.h"
#include "common/updates/clan_result_update.h"
#include "common/updates/clan_review_update.h"
#include "server/world/world.h"

std::vector<std::unique_ptr<GameUpdate>> ClanCommand::execute(World& world) {
    std::vector<std::unique_ptr<GameUpdate>> updates;

    switch (action) {
        case ClanAction::FOUND: {
            ClanResult r = world.found_clan(player_id, arg);
            updates.push_back(std::make_unique<ClanResultUpdate>(player_id, action, r));
            break;
        }
        case ClanAction::JOIN_REQUEST: {
            ClanResult r = world.request_join_clan(player_id, arg);
            updates.push_back(std::make_unique<ClanResultUpdate>(player_id, action, r));
            if (r.status == ClanActionStatus::SUCCESS && r.other_player_id != 0) {
                updates.push_back(std::make_unique<ClanResultUpdate>(r.other_player_id, action, r));
            }
            break;
        }
        case ClanAction::REVIEW: {
            ClanReviewResult rr = world.review_clan(player_id);
            if (rr.status == ClanActionStatus::SUCCESS) {
                updates.push_back(std::make_unique<ClanReviewUpdate>(player_id, rr.clan_name,
                                                                     rr.members, rr.pending));
            } else {
                ClanResult err(rr.status);
                updates.push_back(std::make_unique<ClanResultUpdate>(player_id, action, err));
            }
            break;
        }
        case ClanAction::ACCEPT: {
            ClanResult r = world.accept_clan_member(player_id, arg);
            updates.push_back(std::make_unique<ClanResultUpdate>(player_id, action, r));
            if (r.status == ClanActionStatus::SUCCESS && r.other_player_id != 0) {
                updates.push_back(std::make_unique<ClanResultUpdate>(r.other_player_id, action, r));
            }
            break;
        }
        case ClanAction::REJECT: {
            ClanResult r = world.reject_clan_member(player_id, arg);
            updates.push_back(std::make_unique<ClanResultUpdate>(player_id, action, r));
            if (r.status == ClanActionStatus::SUCCESS && r.other_player_id != 0) {
                updates.push_back(std::make_unique<ClanResultUpdate>(r.other_player_id, action, r));
            }
            break;
        }
        case ClanAction::BAN: {
            ClanResult r = world.ban_clan_member(player_id, arg);
            updates.push_back(std::make_unique<ClanResultUpdate>(player_id, action, r));
            if (r.status == ClanActionStatus::SUCCESS && r.other_player_id != 0) {
                updates.push_back(std::make_unique<ClanResultUpdate>(r.other_player_id, action, r));
            }
            break;
        }
        case ClanAction::KICK: {
            ClanResult r = world.kick_clan_member(player_id, arg);
            updates.push_back(std::make_unique<ClanResultUpdate>(player_id, action, r));
            if (r.status == ClanActionStatus::SUCCESS && r.other_player_id != 0) {
                updates.push_back(std::make_unique<ClanResultUpdate>(r.other_player_id, action, r));
            }
            break;
        }
        case ClanAction::LEAVE: {
            ClanResult r = world.leave_clan(player_id);
            updates.push_back(std::make_unique<ClanResultUpdate>(player_id, action, r));
            break;
        }
    }

    return updates;
}
