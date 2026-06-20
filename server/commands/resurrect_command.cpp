#include "common/commands/resurrect_command.h"

#include <vector>

#include "server/world/world.h"

std::vector<std::unique_ptr<GameUpdate>> ResurrectCommand::execute(World& world) {
    world.start_resurrection(player_id);
    return {};
}
