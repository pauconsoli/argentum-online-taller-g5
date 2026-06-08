#include "server/loader/item_registry_loader.h"

#include <stdexcept>
#include <string>

#include <toml++/toml.hpp>

#include "server/game/items/item_registry.h"

void ItemRegistryLoader::load(const std::string& config_path) {
    auto root = toml::parse_file(config_path);
    auto& registry = ItemRegistry::get_instance();

    const auto* items_table = root["items"].as_table();
    if (!items_table) {
        throw std::runtime_error("items.toml: falta la sección [items]");
    }

    for (auto&& [key, value] : *items_table) {
        const auto* item_table = value.as_table();
        if (!item_table)
            continue;

        ItemRegistry::ItemTemplate tpl;
        if (auto val = (*item_table)["name"].value<std::string>())
            tpl.name = *val;
        if (auto val = (*item_table)["type"].value<std::string>())
            tpl.type = *val;

        if (tpl.name.empty() || tpl.type.empty()) {
            throw std::runtime_error("items.toml: item inválido, falta name o type en " +
                                     std::string(key.str()));
        }

        if (tpl.type == "weapon" || tpl.type == "magic_weapon") {
            tpl.min_damage = (*item_table)["min_damage"].value_or(0);
            tpl.max_damage = (*item_table)["max_damage"].value_or(0);
            tpl.ranged = (*item_table)["ranged"].value_or(false);
        }
        if (tpl.type == "defensive") {
            tpl.min_defense = (*item_table)["min_defense"].value_or(0);
            tpl.max_defense = (*item_table)["max_defense"].value_or(0);
            if (auto val = (*item_table)["slot"].value<std::string>())
                tpl.slot = *val;
        }
        if (tpl.type == "consumable") {
            if (auto val = (*item_table)["consumable_type"].value<std::string>())
                tpl.consumable_type = *val;
            tpl.restore = (*item_table)["restore"].value_or(0);
        }
        if (tpl.type == "magic_weapon") {
            if (auto val = (*item_table)["spell_name"].value<std::string>())
                tpl.spell_name = *val;
            tpl.min_heal = (*item_table)["min_heal"].value_or(0);
            tpl.max_heal = (*item_table)["max_heal"].value_or(0);
            tpl.mana_cost = (*item_table)["mana_cost"].value_or(0);
        }

        registry.add_template(tpl.name, tpl);
    }
}
