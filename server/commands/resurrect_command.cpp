#include "common/commands/resurrect_command.h"

#include <vector>

#include "common/protocol_constants.h"
#include "common/updates/chat_message_update.h"
#include "common/updates/error_update.h"
#include "server/game/player.h"
#include "server/world/world.h"

std::vector<std::unique_ptr<GameUpdate>> ResurrectCommand::execute(World& world) {
    std::vector<std::unique_ptr<GameUpdate>> updates;

    bool success = world.start_resurrection(player_id);

    if (success) {
        updates.push_back(std::make_unique<ChatMessageUpdate>(
            player_id, "Comenzando el viaje hacia el sanador. Por favor espera..."));
    } else {
        updates.push_back(std::make_unique<ErrorUpdate>(
            player_id, ProtocolError::COMMAND_NOT_ALLOWED, "No se pudo resucitar"));
    }

    return updates;
}
