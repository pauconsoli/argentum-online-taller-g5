#include "common/commands/cheat_command.h"

#include <string>
#include <vector>

#include "common/cheat_type.h"
#include "common/updates/death_update.h"
#include "common/updates/system_msg_update.h"
#include "server/world/world.h"

static std::string cheat_msg(CheatType cheat, bool died) {
    switch (cheat) {
        case CheatType::HEAL_FULL:
            return "[CHEAT] HP restaurado al maximo";
        case CheatType::RESTORE_MANA:
            return "[CHEAT] Mana restaurado al maximo";
        case CheatType::DIE:
            return died ? "[CHEAT] Moriste instantaneamente" : "";
        case CheatType::LEVEL_UP:
            return "[CHEAT] Subiste de nivel";
        case CheatType::GIVE_GOLD:
            return "[CHEAT] Recibiste 10000 de oro";
    }
    return "";
}

std::vector<std::unique_ptr<GameUpdate>> CheatCommand::execute(World& world) {
    std::vector<std::unique_ptr<GameUpdate>> updates;

    CheatResult result = world.apply_cheat(player_id, cheat_type);
    if (!result.success)
        return updates;

    if (result.player_died)
        updates.push_back(std::make_unique<DeathUpdate>(player_id, player_id));

    std::string msg = cheat_msg(cheat_type, result.player_died);
    if (!msg.empty())
        updates.push_back(std::make_unique<SystemMsgUpdate>(player_id, std::move(msg)));

    return updates;
}
