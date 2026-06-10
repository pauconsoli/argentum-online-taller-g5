#include "common/commands/move_command.h"

#include <stdexcept>
#include <vector>

#include "common/protocol_constants.h"
#include "common/updates/error_update.h"
#include "common/updates/moved_update.h"
#include "server/game/player.h"
#include "server/world/world.h"

std::vector<std::unique_ptr<GameUpdate>> MoveCommand::execute(World& world) {
    std::vector<std::unique_ptr<GameUpdate>> updates;
    Player* player = world.get_player(player_id);
    if (!player) {
        updates.push_back(std::make_unique<ErrorUpdate>(
            player_id, ProtocolError::COMMAND_NOT_ALLOWED, "Jugador no encontrado"));
        return updates;
    }

    player->stop_meditating();

    bool success = world.move_player(player_id, direction);

    if (success) {
        updates.push_back(std::make_unique<MovedUpdate>(player_id, player->get_position()));
    } else {
        updates.push_back(std::make_unique<ErrorUpdate>(
            player_id, ProtocolError::COMMAND_NOT_ALLOWED, "No puedes moverte en esa dirección"));
    }

    return updates;
}
