#include "common/commands/move_command.h"

#include <stdexcept>

#include "common/protocol_constants.h"
#include "common/updates/error_update.h"
#include "common/updates/moved_update.h"
#include "server/game/player.h"
#include "server/world/world.h"

std::unique_ptr<GameUpdate> MoveCommand::execute(World& world) {
    Player* player = world.get_player(player_id);
    if (!player) {
        return nullptr;
    }

    player->stop_meditating();

    try {
        world.move_player(player_id, direction);
        return std::make_unique<MovedUpdate>(player_id, player->get_position());
    } catch (const std::exception& e) {
        return std::make_unique<ErrorUpdate>(ProtocolError::COMMAND_NOT_ALLOWED, e.what());
    }
}
