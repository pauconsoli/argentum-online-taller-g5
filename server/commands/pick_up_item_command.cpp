#include "common/commands/pick_up_item_command.h"

#include <vector>

#include "common/updates/inventory_update.h"
#include "server/game/player.h"
#include "server/world/world.h"

std::vector<std::unique_ptr<GameUpdate>> PickUpItemCommand::execute(World& world) {
    std::vector<std::unique_ptr<GameUpdate>> updates;

    if (!world.pick_up_item(player_id))
        return updates;

    Player* player = world.get_player(player_id);
    std::vector<InventorySlotData> items_data;
    for (const auto& slot : player->get_inventory().get_slots()) {
        if (slot.item) {
            items_data.push_back({slot.item->get_name(), static_cast<uint32_t>(slot.quantity),
                                  slot.equipped_slot.has_value()});
        } else {
            items_data.push_back({"", 0, false});
        }
    }

    updates.push_back(
        std::make_unique<InventoryUpdate>(player_id, std::move(items_data), player->get_gold()));

    return updates;
}
