#include "server/loader/npc_registry_loader.h"

#include <stdexcept>
#include <string>
#include <utility>

#include <toml++/toml.hpp>

#include "server/game/npcs/npc_registry.h"
#include "server/game/npcs/npc_type.h"

static NPCType parse_npc_type(const std::string& id) {
    if (id == "goblin")
        return NPCType::GOBLIN;
    if (id == "goblin_executioner")
        return NPCType::GOBLIN_EXECUTIONER;
    if (id == "spider")
        return NPCType::SPIDER;
    if (id == "spider_giant")
        return NPCType::SPIDER_GIANT;
    if (id == "skeleton")
        return NPCType::SKELETON;
    if (id == "skeleton_warrior")
        return NPCType::SKELETON_WARRIOR;
    if (id == "zombie")
        return NPCType::ZOMBIE;
    if (id == "orc")
        return NPCType::ORC;
    if (id == "golem")
        return NPCType::GOLEM;
    if (id == "golem_iron")
        return NPCType::GOLEM_IRON;
    return NPCType::UNKNOWN;
}

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
        tpl.npc_type = parse_npc_type(tpl.id);
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
