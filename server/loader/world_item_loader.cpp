#include "server/loader/world_item_loader.h"

#include <stdexcept>
#include <string>
#include <utility>

#include <toml++/toml.hpp>

#include "server/game/items/item_registry.h"

void WorldItemLoader::load_ground_items(World& world, const std::string& map_config_path) {
    auto root = toml::parse_file(map_config_path);

    auto* ground_items_array = root["ground_items"].as_array();
    if (!ground_items_array) {
        return;  // No hay items definidos para el suelo
    }

    auto& registry = ItemRegistry::get_instance();

    for (auto& elem : *ground_items_array) {
        auto* item_table = elem.as_table();
        if (!item_table)
            continue;

        std::string type = (*item_table)["type"].value_or("");
        int x = (*item_table)["x"].value_or(0);
        int y = (*item_table)["y"].value_or(0);
        Position pos{x, y};

        if (type == "item") {
            std::string name = (*item_table)["name"].value_or("");
            int quantity = (*item_table)["quantity"].value_or(1);

            auto item = registry.create_item(name);
            world.add_ground_item(pos, GroundItem{0, std::move(item), quantity});
        } else if (type == "gold") {
            uint64_t amount = (*item_table)["amount"].value_or(0);
            if (amount > 0) {
                world.add_ground_item(pos, GroundItem{amount, nullptr, 0});
            }
        }
    }
}
