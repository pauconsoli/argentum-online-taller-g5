#include "server/loader/npc_registry_loader.h"

#include <stdexcept>
#include <string>
#include <utility>

#include <toml++/toml.hpp>

#include "server/game/npcs/npc_registry.h"

void NPCRegistryLoader::load(const std::string& config_path) {
    auto root = toml::parse_file(config_path);
    auto& registry = NPCRegistry::get_instance();

    for (auto&& [key, value] : root) {
        const auto* npc_table = value.as_table();
        if (!npc_table)
            continue;

        NPCTemplate tpl;
        tpl.id = std::string(key.str());
        tpl.name = (*npc_table)["name"].value_or("");
        tpl.level = (*npc_table)["level"].value_or(0);
        tpl.max_hp = (*npc_table)["max_hp"].value_or(0);
        tpl.defense = (*npc_table)["defense"].value_or(0);
        tpl.agility = (*npc_table)["agility"].value_or(0);
        tpl.min_damage = (*npc_table)["min_damage"].value_or(0);
        tpl.max_damage = (*npc_table)["max_damage"].value_or(0);
        tpl.attack_range = (*npc_table)["attack_range"].value_or(0);

        auto* zones_array = (*npc_table)["zones"].as_array();
        if (zones_array) {
            for (auto& zone_val : *zones_array) {
                if (auto zone_str = zone_val.value<std::string>()) {
                    tpl.zones.push_back(*zone_str);
                }
            }
        }

        if (tpl.name.empty()) {
            throw std::runtime_error("npcs.toml: npc inválido, falta name en " + tpl.id);
        }

        registry.add_template(tpl.id, std::move(tpl));
    }
}
