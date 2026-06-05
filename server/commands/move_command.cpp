#include "common/commands/move_command.h"

#include "common/updates/moved_update.h"
#include "server/game/player.h"
#include "server/world/world.h"

std::unique_ptr<GameUpdate> MoveCommand::execute(World& world) {
    Player* player = world.get_player(player_id);
    if (!player) {
        return nullptr;
    }

    player->stop_meditating();

    if (world.move_player(player_id, direction)) {
        return std::make_unique<MovedUpdate>(player_id, player->get_position());
    }

    return nullptr;
}
