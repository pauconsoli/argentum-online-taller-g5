#include "common/commands/resurrect_command.h"

#include <vector>

#include "common/resurrect_status.h"
#include "common/updates/revive_update.h"
#include "server/world/world.h"

std::vector<std::unique_ptr<GameUpdate>> ResurrectCommand::execute(World& world) {
    std::vector<std::unique_ptr<GameUpdate>> updates;
    ResurrectStatus status = world.start_resurrection(player_id);
    updates.push_back(std::make_unique<ReviveUpdate>(player_id, status));
    return updates;
}
