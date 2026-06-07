#include "common/commands/attack_command.h"

#include <stdexcept>
#include <vector>

#include "common/protocol_constants.h"
#include "common/updates/attack_update.h"
#include "common/updates/death_update.h"
#include "common/updates/error_update.h"
#include "server/game/character.h"
#include "server/game/player.h"
#include "server/world/world.h"

std::vector<std::unique_ptr<GameUpdate>> AttackCommand::execute(World& world) {
    std::vector<std::unique_ptr<GameUpdate>> updates;
    Player* attacker = world.get_player(player_id);
    Character* target = world.get_character(target_id);

    if (!attacker || !target) {
        updates.push_back(std::make_unique<ErrorUpdate>(
            player_id, ProtocolError::COMMAND_NOT_ALLOWED, "Invalid attacker or target"));
        return updates;
    }

    try {
        AttackResult result = world.attack(player_id, target_id);
        updates.push_back(std::make_unique<AttackUpdate>(result));

        if (result.target_died) {
            updates.push_back(std::make_unique<DeathUpdate>(target_id, player_id));
        }
    } catch (const std::exception& e) {  // validaciones
        updates.push_back(
            std::make_unique<ErrorUpdate>(player_id, ProtocolError::COMMAND_NOT_ALLOWED, e.what()));
    }
    return updates;
}
