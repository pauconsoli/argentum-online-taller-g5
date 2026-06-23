#include "common/commands/attack_command.h"

#include <vector>

#include "common/updates/attack_update.h"
#include "common/updates/death_update.h"
#include "common/updates/inventory_update.h"
#include "server/game/character.h"
#include "server/game/player.h"
#include "server/world/world.h"

std::vector<std::unique_ptr<GameUpdate>> AttackCommand::execute(World& world) {
    std::vector<std::unique_ptr<GameUpdate>> updates;

    Player* attacker = world.get_player(player_id);
    AttackResult result;
    if (attacker && attacker->is_healing_attack()) {
        result = world.heal(player_id, target_id);
    } else {
        result = world.attack(player_id, target_id);
    }

    updates.push_back(std::make_unique<AttackUpdate>(result, player_id));
    if (result.status != AttackStatus::SUCCESS) {
        return updates;
    }

    if (result.attacker_id != result.target_id) {
        updates.push_back(std::make_unique<AttackUpdate>(result, result.target_id));
    }

    if (result.damage > 0 && result.target_clan_id != 0) {
        Clan* target_clan = world.get_clan(result.target_clan_id);
        if (target_clan) {
            for (uint32_t member_id : target_clan->get_members()) {
                if (member_id != result.attacker_id && member_id != result.target_id) {
                    updates.push_back(std::make_unique<AttackUpdate>(result, member_id));
                }
            }
        }
    }

    if (result.target_died) {
        updates.push_back(std::make_unique<DeathUpdate>(target_id, player_id));

        Player* dead_player = world.get_player(target_id);
        if (dead_player) {
            std::vector<InventorySlotData> items_data;
            for (const auto& slot : dead_player->get_inventory().get_slots()) {
                if (slot.item) {
                    items_data.push_back({slot.item->get_name(),
                                          static_cast<uint32_t>(slot.quantity),
                                          slot.equipped_slot.has_value()});
                } else {
                    items_data.push_back({"", 0, false});
                }
            }
            updates.push_back(std::make_unique<InventoryUpdate>(target_id, std::move(items_data),
                                                                dead_player->get_gold()));
        }
    }

    return updates;
}
