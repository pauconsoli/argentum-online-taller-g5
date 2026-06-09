#include "common/commands/attack_command.h"

#include <stdexcept>
#include <vector>

#include "common/protocol_constants.h"
#include "common/updates/attack_update.h"
#include "common/updates/death_update.h"
#include "common/updates/error_update.h"
#include "common/updates/inventory_update.h"
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

    AttackResult result = world.attack(player_id, target_id);
    if (result.status != AttackStatus::SUCCESS) {
        std::string error_msg;
        switch (result.status) {
            case AttackStatus::NO_MANA:
                error_msg = "No tienes suficiente mana";
                break;
            case AttackStatus::OUT_OF_RANGE:
                error_msg = "El objetivo está fuera de rango";
                break;
            case AttackStatus::INVALID_TARGET:
                error_msg = "Objetivo inválido";
                break;
            case AttackStatus::DEAD:
                error_msg = "Jugador u objetivo muertos";
                break;
            default:
                error_msg = "Ataque inválido";
                break;
        }
        updates.push_back(std::make_unique<ErrorUpdate>(
            player_id, ProtocolError::COMMAND_NOT_ALLOWED, error_msg));
        return updates;
    }

    updates.push_back(std::make_unique<AttackUpdate>(result));

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
