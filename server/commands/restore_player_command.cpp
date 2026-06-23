#include "restore_player_command.h"

#include <iostream>
#include <stdexcept>
#include <utility>

#include "common/position.h"
#include "common/protocol_constants.h"
#include "common/updates/error_update.h"
#include "common/updates/inventory_update.h"
#include "common/updates/spawned_update.h"
#include "server/game/game_formulas.h"
#include "server/game/inventory.h"
#include "server/game/items/item.h"
#include "server/game/items/item_registry.h"
#include "server/game/player.h"
#include "server/game/player_class.h"
#include "server/game/player_race.h"
#include "server/world/world.h"

std::vector<std::unique_ptr<GameUpdate>> RestorePlayerCommand::execute(World& world) {
    std::vector<std::unique_ptr<GameUpdate>> updates;

    if (world.player_exists(player_id)) {
        updates.push_back(std::make_unique<ErrorUpdate>(
            player_id, ProtocolError::COMMAND_NOT_ALLOWED,
            "El jugador ya está conectado en este match"));
        return updates;
    }

    try {
        PlayerRace race = static_cast<PlayerRace>(save.race);
        PlayerClass klass = static_cast<PlayerClass>(save.klass);
        Position pos{save.pos_x, save.pos_y};

        auto player = GameFormulas::create_initial_player(player_id, save.nick, race, klass, pos);

        if (save.level > 1) {
            player->set_level(static_cast<int>(save.level));
        }

        int max_hp = save.max_hp > 0 ? save.max_hp : 1;
        int max_mp = save.max_mp > 0 ? save.max_mp : 0;
        player->set_initial_stats(max_hp, max_mp);

        if (save.gold > 0) {
            player->add_gold(save.gold);
        }
        if (save.xp > 0) {
            player->add_experience(save.xp);
        }

        for (const auto& slot_save: save.inventory) {
            if (slot_save.item_name.empty()) {
                continue;
            }
            auto item = ItemRegistry::get_instance().create_item(slot_save.item_name);
            if (!item) {
                std::cerr << "[RESTORE] Item '" << slot_save.item_name
                          << "' no existe en el registry. Se saltea.\n";
                continue;
            }

            Item* item_ref = item.get();
            int qty = std::max(1, static_cast<int>(slot_save.quantity));

            if (!player->get_inventory().add_item(std::move(item), qty)) {
                std::cerr << "[RESTORE] No se pudo agregar '" << slot_save.item_name
                          << "' al inventario de " << save.nick << ".\n";
                continue;
            }

            if (slot_save.is_equipped) {
                player->get_inventory().equip(*item_ref, *player);
            }
        }

        std::vector<InventorySlotData> items_data;
        for (const auto& slot: player->get_inventory().get_slots()) {
            if (slot.item) {
                items_data.push_back({slot.item->get_name(),
                                      static_cast<uint32_t>(slot.quantity),
                                      slot.equipped_slot.has_value()});
            } else {
                items_data.push_back({"", 0, false});
            }
        }

        uint64_t final_gold = player->get_gold();

        world.add_player(std::move(player));

        updates.push_back(std::make_unique<SpawnedUpdate>(player_id, save.nick, race, klass, pos));
        updates.push_back(
            std::make_unique<InventoryUpdate>(player_id, std::move(items_data), final_gold));

    } catch (const std::exception& e) {
        std::cerr << "[RESTORE] Error restaurando jugador " << save.nick << ": "
                  << e.what() << "\n";
        updates.push_back(std::make_unique<ErrorUpdate>(
            player_id, ProtocolError::COMMAND_NOT_ALLOWED,
            std::string("No se pudo restaurar el personaje: ") + e.what()));
    }

    return updates;
}
