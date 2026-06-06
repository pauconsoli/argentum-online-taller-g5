#include "common/commands/attack_command.h"

#include "common/updates/attack_update.h"
#include "server/game/game_formulas.h"
#include "server/game/player.h"
#include "server/world/world.h"

std::unique_ptr<GameUpdate> AttackCommand::execute(World& world) {
    Player* attacker = world.get_player(player_id);
    Player* target = world.get_player(target_id);

    if (!attacker || !target)
        return nullptr;

    AttackResult result = world.attack_player(player_id, target_id);

    return std::make_unique<AttackUpdate>(result);
}
