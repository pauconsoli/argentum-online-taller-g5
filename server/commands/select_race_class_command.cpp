#include "common/commands/select_race_class_command.h"

#include <iostream>
#include <stdexcept>
#include <vector>

#include "common/updates/inventory_update.h"
#include "common/updates/spawned_update.h"
#include "server/game/game_formulas.h"
#include "server/game/player.h"
#include "server/world/world.h"

std::vector<std::unique_ptr<GameUpdate>> SelectRaceClassCommand::execute(World& world) {
    std::vector<std::unique_ptr<GameUpdate>> updates;

    if (world.player_exists(player_id))
        return updates;

    try {
        Position spawn = world.get_spawn_position();
        auto player = GameFormulas::create_initial_player(player_id, nick, race, klass, spawn);

        uint64_t initial_gold = player->get_gold();

        std::vector<InventorySlotData> items_data;
        for (const auto& slot : player->get_inventory().get_slots()) {
            if (slot.item) {
                items_data.push_back({slot.item->get_name(), static_cast<uint32_t>(slot.quantity),
                                      slot.equipped_slot.has_value()});
            } else {
                items_data.push_back({"", 0, false});
            }
        }

        world.add_player(std::move(player));

        updates.push_back(std::make_unique<SpawnedUpdate>(player_id, nick, race, klass, spawn));
        updates.push_back(
            std::make_unique<InventoryUpdate>(player_id, std::move(items_data), initial_gold));

    } catch (const std::exception& e) {
        std::cerr << "[SelectRaceClass] Error spawning player " << player_id << ": " << e.what()
                  << "\n";
    }

    return updates;
}
