#include "common/commands/attack_command.h"

#include <stdexcept>

#include "common/protocol_constants.h"
#include "common/updates/attack_update.h"
#include "common/updates/error_update.h"
#include "server/game/character.h"
#include "server/game/player.h"
#include "server/world/world.h"

std::unique_ptr<GameUpdate> AttackCommand::execute(World& world) {
    Player* attacker = world.get_player(player_id);
    Character* target = world.get_character(target_id);

    if (!attacker || !target)
        return nullptr;

    try {
        AttackResult result = world.attack(player_id, target_id);
        return std::make_unique<AttackUpdate>(result);
    } catch (const std::exception& e) {  // validaciones
        return std::make_unique<ErrorUpdate>(ProtocolError::COMMAND_NOT_ALLOWED, e.what());
    }
}
