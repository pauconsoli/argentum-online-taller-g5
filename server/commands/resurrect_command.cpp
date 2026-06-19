#include "common/commands/resurrect_command.h"

#include <vector>

#include "common/protocol_constants.h"
#include "common/updates/error_update.h"
#include "common/updates/system_msg_update.h"
#include "server/game/player.h"
#include "server/world/world.h"

std::vector<std::unique_ptr<GameUpdate>> ResurrectCommand::execute(World& world) {
    std::vector<std::unique_ptr<GameUpdate>> updates;

    bool success = world.start_resurrection(player_id);

    if (success) {
        updates.push_back(std::make_unique<SystemMsgUpdate>(
            player_id, "Comenzando la resurrección. Estarás inmovilizado durante el proceso..."));
    } else {
        updates.push_back(std::make_unique<ErrorUpdate>(
            player_id, ProtocolError::COMMAND_NOT_ALLOWED, "No se pudo resucitar"));
    }

    return updates;
}
