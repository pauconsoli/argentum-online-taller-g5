#include "common/commands/move_command.h"

#include "server/game/player.h"
#include "server/world/world.h"

std::unique_ptr<GameUpdate> MoveCommand::execute(World& world) {
    Player* player = world.get_player(player_id);
    if (player) {
        player->stop_meditating();
    }
    return world.move_player(player_id, direction);
}
