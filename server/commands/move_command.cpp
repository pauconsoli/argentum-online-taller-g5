#include "common/commands/move_command.h"

#include <vector>

#include "common/updates/moved_update.h"
#include "server/game/player.h"
#include "server/world/world.h"

std::vector<std::unique_ptr<GameUpdate>> MoveCommand::execute(World& world) {
    std::vector<std::unique_ptr<GameUpdate>> updates;
    Player* player = world.get_player(player_id);
    if (!player)
        return updates;

    player->stop_meditating();

    if (world.move_character(player_id, direction)) {
        updates.push_back(std::make_unique<MovedUpdate>(player_id, player->get_position()));
    }

    return updates;
}
