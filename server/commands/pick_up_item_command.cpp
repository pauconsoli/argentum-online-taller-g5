#include "common/commands/pick_up_item_command.h"

#include <vector>

#include "common/protocol_constants.h"
#include "common/updates/error_update.h"
#include "common/updates/inventory_update.h"
#include "server/game/player.h"
#include "server/world/world.h"

std::unique_ptr<GameUpdate> PickUpItemCommand::execute(World& world) {
    try {
        world.pick_up_item(player_id);

        Player* player = world.get_player(player_id);
        std::vector<InventorySlotData> items_data;
        for (const auto& slot : player->get_inventory().get_slots()) {
            items_data.push_back({slot.item->get_name(), static_cast<uint32_t>(slot.quantity),
                                  slot.equipped_slot.has_value()});
        }

        return std::make_unique<InventoryUpdate>(player_id, std::move(items_data),
                                                 player->get_gold());
    } catch (const std::exception& e) {
        return std::make_unique<ErrorUpdate>(player_id, ProtocolError::COMMAND_NOT_ALLOWED,
                                             e.what());
    }
}
