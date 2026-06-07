#include "common/commands/pick_up_item_command.h"

#include "common/protocol_constants.h"
#include "common/updates/error_update.h"
#include "server/game/player.h"
#include "server/world/world.h"

std::unique_ptr<GameUpdate> PickUpItemCommand::execute(World& world) {
    try {
        world.pick_up_item(player_id);

        // Player* player = world.get_player(player_id);
        //  return std::make_unique<InventoryUpdate>(...);
    } catch (const std::exception& e) {
        return std::make_unique<ErrorUpdate>(ProtocolError::COMMAND_NOT_ALLOWED, e.what());
    }
}
