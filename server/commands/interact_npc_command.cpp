#include "common/commands/interact_npc_command.h"

#include <vector>

#include "common/npc_interact_result.h"
#include "common/npc_type.h"
#include "common/updates/catalog_update.h"
#include "common/updates/inventory_update.h"
#include "common/updates/npc_interact_update.h"
#include "server/game/npcs/npc.h"
#include "server/game/player.h"
#include "server/world/world.h"

std::vector<std::unique_ptr<GameUpdate>> InteractNPCCommand::execute(World& world) {
    std::vector<std::unique_ptr<GameUpdate>> updates;

    InteractResult result = world.interact_with_npc(player_id, npc_id, type, arg, amount);

    if (type == NPCInteraction::LIST && result.status == InteractStatus::SUCCESS) {
        NPC* npc = world.get_npc(npc_id);
        bool is_vault = npc && npc->get_type() == NPCType::BANKER;
        updates.push_back(std::make_unique<CatalogUpdate>(player_id, result.catalog,
                                                          result.gold_amount, is_vault));
        return updates;
    }

    updates.push_back(std::make_unique<NpcInteractUpdate>(player_id, type, result));

    if (result.status == InteractStatus::SUCCESS) {
        if (Player* player = world.get_player(player_id)) {
            std::vector<InventorySlotData> items_data;
            for (const auto& slot : player->get_inventory().get_slots()) {
                if (slot.item) {
                    items_data.push_back({slot.item->get_name(),
                                          static_cast<uint32_t>(slot.quantity),
                                          slot.equipped_slot.has_value()});
                } else {
                    items_data.push_back({"", 0, false});
                }
            }
            updates.push_back(std::make_unique<InventoryUpdate>(player_id, std::move(items_data),
                                                                player->get_gold()));
        }
    }

    return updates;
}
