#include "common/commands/meditate_command.h"

#include <vector>

#include "common/meditate_status.h"
#include "common/protocol_constants.h"
#include "common/updates/error_update.h"
#include "server/world/world.h"

std::vector<std::unique_ptr<GameUpdate>> MeditateCommand::execute(World& world) {
    std::vector<std::unique_ptr<GameUpdate>> updates;

    switch (world.meditate(player_id)) {
        case MeditateStatus::SUCCESS:
            break;
        case MeditateStatus::PLAYER_DEAD:
            updates.push_back(std::make_unique<ErrorUpdate>(
                player_id, ProtocolError::COMMAND_NOT_ALLOWED, "Estás muerto, no podes meditar"));
            break;
        case MeditateStatus::NO_MANA:
            updates.push_back(std::make_unique<ErrorUpdate>(
                player_id, ProtocolError::COMMAND_NOT_ALLOWED, "Tu clase no tiene mana"));
            break;
    }

    return updates;
}
