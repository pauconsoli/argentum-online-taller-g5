#ifndef CLAN_RESULT_H
#define CLAN_RESULT_H

#include <cstdint>
#include <string>
#include <utility>

#include "common/clan/clan_action_status.h"

struct ClanResult {
    ClanActionStatus status = ClanActionStatus::SUCCESS;
    std::string clan_name;

    std::string actor_nick;  // el que ejecutó la acción

    uint32_t other_player_id = 0;  // el otro jugador involucrado
    std::string other_nick;

    // constructores para no repetir el orden de miembros en cada return
    ClanResult() = default;
    explicit ClanResult(ClanActionStatus status): status(status) {}
    ClanResult(ClanActionStatus status, std::string clan_name):
        status(status), clan_name(std::move(clan_name)) {}
    ClanResult(ClanActionStatus status, std::string clan_name, std::string actor_nick):
        status(status), clan_name(std::move(clan_name)), actor_nick(std::move(actor_nick)) {}
    ClanResult(ClanActionStatus status, std::string clan_name, std::string actor_nick,
               uint32_t other_player_id, std::string other_nick):
        status(status),
        clan_name(std::move(clan_name)),
        actor_nick(std::move(actor_nick)),
        other_player_id(other_player_id),
        other_nick(std::move(other_nick)) {}
};

#endif
