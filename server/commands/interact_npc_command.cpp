#include "common/commands/interact_npc_command.h"

#include <string>
#include <vector>

#include "common/npc_interact_result.h"
#include "common/protocol_constants.h"
#include "common/updates/catalog_update.h"
#include "common/updates/error_update.h"
#include "common/updates/inventory_update.h"
#include "common/updates/system_msg_update.h"
#include "server/game/player.h"
#include "server/world/world.h"

std::vector<std::unique_ptr<GameUpdate>> InteractNPCCommand::execute(World& world) {
    std::vector<std::unique_ptr<GameUpdate>> updates;

    InteractResult result = world.interact_with_npc(player_id, npc_id, type, arg, amount);

    switch (result.status) {
        case InteractStatus::SUCCESS: {
            if (type == NPCInteraction::LIST) {
                updates.push_back(
                    std::make_unique<CatalogUpdate>(player_id, result.catalog, result.gold_amount));
            } else {
                std::string msg = "Interacción exitosa";
                if (type == NPCInteraction::BUY) {
                    msg = "Compraste " + result.item_name + " por " +
                          std::to_string(result.gold_amount) + " de oro";
                } else if (type == NPCInteraction::SELL) {
                    msg = "Vendiste " + result.item_name + " por " +
                          std::to_string(result.gold_amount) + " de oro";
                } else if (type == NPCInteraction::DEPOSIT_GOLD) {
                    msg = "Depositaste " + std::to_string(result.gold_amount) + " de oro";
                } else if (type == NPCInteraction::WITHDRAW_GOLD) {
                    msg = "Retiraste " + std::to_string(result.gold_amount) + " de oro";
                } else if (type == NPCInteraction::DEPOSIT_ITEM) {
                    msg = "Depositaste " + result.item_name;
                } else if (type == NPCInteraction::WITHDRAW_ITEM) {
                    msg = "Retiraste " + result.item_name;
                } else if (type == NPCInteraction::HEAL) {
                    msg = "Has sido curado completamente";
                } else if (type == NPCInteraction::RESURRECT) {
                    msg = "Has sido resucitado";
                }

                updates.push_back(std::make_unique<SystemMsgUpdate>(player_id, msg));

                // actualizo el inventario en caso de que haya cambiado por la
                // compra/venta/deposito/retiro
                if (Player* player = world.get_player(player_id)) {
                    std::vector<InventorySlotData> items_data;
                    for (const auto& i_slot : player->get_inventory().get_slots()) {
                        if (i_slot.item) {
                            items_data.push_back({i_slot.item->get_name(),
                                                  static_cast<uint32_t>(i_slot.quantity),
                                                  i_slot.equipped_slot.has_value()});
                        } else {
                            items_data.push_back({"", 0, false});
                        }
                    }
                    updates.push_back(std::make_unique<InventoryUpdate>(
                        player_id, std::move(items_data), player->get_gold()));
                }
            }
            break;
        }
        case InteractStatus::INSUFFICIENT_GOLD:
            updates.push_back(std::make_unique<ErrorUpdate>(
                player_id, ProtocolError::COMMAND_NOT_ALLOWED, "No tenés oro suficiente"));
            break;
        case InteractStatus::INVENTORY_FULL:
            updates.push_back(std::make_unique<ErrorUpdate>(player_id,
                                                            ProtocolError::COMMAND_NOT_ALLOWED,
                                                            "No tenés espacio en el inventario"));
            break;
        case InteractStatus::ITEM_NOT_FOUND:
            updates.push_back(std::make_unique<ErrorUpdate>(
                player_id, ProtocolError::COMMAND_NOT_ALLOWED, "Ítem no encontrado"));
            break;
        case InteractStatus::NOT_ALLOWED:
            updates.push_back(std::make_unique<ErrorUpdate>(
                player_id, ProtocolError::COMMAND_NOT_ALLOWED, "El NPC no puede hacer eso"));
            break;
        case InteractStatus::PLAYER_DEAD:
            updates.push_back(std::make_unique<ErrorUpdate>(
                player_id, ProtocolError::COMMAND_NOT_ALLOWED, "¡Estás muerto!"));
            break;
        case InteractStatus::PLAYER_NOT_DEAD:
            updates.push_back(std::make_unique<ErrorUpdate>(
                player_id, ProtocolError::COMMAND_NOT_ALLOWED, "No estás muerto"));
            break;
        case InteractStatus::ALREADY_FULL:
            updates.push_back(std::make_unique<ErrorUpdate>(
                player_id, ProtocolError::COMMAND_NOT_ALLOWED, "Ya tenés salud/maná al máximo"));
            break;
        case InteractStatus::INVALID_AMOUNT:
            updates.push_back(std::make_unique<ErrorUpdate>(
                player_id, ProtocolError::COMMAND_NOT_ALLOWED, "Cantidad de oro inválida"));
            break;
        case InteractStatus::OUT_OF_RANGE:
            updates.push_back(std::make_unique<ErrorUpdate>(
                player_id, ProtocolError::COMMAND_NOT_ALLOWED, "Estás demasiado lejos del NPC"));
            break;
        case InteractStatus::INVALID_TARGET:
            updates.push_back(std::make_unique<ErrorUpdate>(
                player_id, ProtocolError::COMMAND_NOT_ALLOWED, "NPC no encontrado"));
            break;
    }

    return updates;
}
