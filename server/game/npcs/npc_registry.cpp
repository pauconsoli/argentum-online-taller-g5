#include "server/game/npcs/npc_registry.h"

#include <utility>

NPCRegistry& NPCRegistry::get_instance() {
    static NPCRegistry instance;
    return instance;
}

void NPCRegistry::add_template(const std::string& id, NPCTemplate&& npc_template) {
    templates.emplace(id, std::move(npc_template));
}

const NPCTemplate* NPCRegistry::get_template(const std::string& id) const {
    auto it = templates.find(id);
    if (it != templates.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<const NPCTemplate*> NPCRegistry::get_all_templates() const {
    std::vector<const NPCTemplate*> all_templates;
    all_templates.reserve(templates.size());
    for (const auto& pair : templates) {
        all_templates.push_back(&pair.second);
    }
    return all_templates;
}
