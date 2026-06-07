#include "common/commands/meditate_command.h"

#include <vector>

#include "common/protocol_constants.h"
#include "common/updates/error_update.h"
#include "server/game/player.h"
#include "server/world/world.h"

std::vector<std::unique_ptr<GameUpdate>> MeditateCommand::execute(World& world) {
    std::vector<std::unique_ptr<GameUpdate>> updates;
    Player* player = world.get_player(player_id);
    if (!player) {
        updates.push_back(std::make_unique<ErrorUpdate>(
            player_id, ProtocolError::COMMAND_NOT_ALLOWED, "Jugador no encontrado"));
        return updates;
    }

    if (player->is_dead()) {
        updates.push_back(std::make_unique<ErrorUpdate>(
            player_id, ProtocolError::COMMAND_NOT_ALLOWED, "Estás muerto, no podes meditar"));
        return updates;
    }

    player->start_meditating();

    if (!player->is_meditating()) {
        updates.push_back(std::make_unique<ErrorUpdate>(
            player_id, ProtocolError::COMMAND_NOT_ALLOWED, "Tu clase no tiene mana"));
    }

    return updates;
}
