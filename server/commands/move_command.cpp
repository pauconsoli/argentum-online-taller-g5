#include "common/commands/move_command.h"

#include "server/world/world.h"

std::unique_ptr<GameUpdate> MoveCommand::execute(World& world) {
    return world.move_player(player_id, direction);
}
