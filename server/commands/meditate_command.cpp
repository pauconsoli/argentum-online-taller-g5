#include "common/commands/meditate_command.h"

#include <vector>

#include "common/meditate_status.h"
#include "common/updates/meditate_update.h"
#include "server/world/world.h"

std::vector<std::unique_ptr<GameUpdate>> MeditateCommand::execute(World& world) {
    std::vector<std::unique_ptr<GameUpdate>> updates;
    MeditateStatus status = world.meditate(player_id);
    updates.push_back(std::make_unique<MeditateUpdate>(player_id, status));
    return updates;
}
