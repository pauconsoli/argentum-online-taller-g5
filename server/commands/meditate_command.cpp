#include "common/commands/meditate_command.h"

#include "common/protocol_constants.h"
#include "common/updates/error_update.h"
#include "server/game/player.h"
#include "server/world/world.h"

std::unique_ptr<GameUpdate> MeditateCommand::execute(World& world) {
    Player* player = world.get_player(player_id);
    if (!player)
        return nullptr;

    if (player->is_dead()) {
        return std::make_unique<ErrorUpdate>(player_id, ProtocolError::COMMAND_NOT_ALLOWED,
                                             "Estás muerto, no puedes meditar.");
    }

    player->start_meditating();

    if (!player->is_meditating()) {
        return std::make_unique<ErrorUpdate>(player_id, ProtocolError::COMMAND_NOT_ALLOWED,
                                             "Tu clase no tiene mana");
    }

    return nullptr;
}
