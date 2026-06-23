#ifndef NPC_REGISTRY_H
#define NPC_REGISTRY_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "server/game/npcs/npc_template.h"

class NPCRegistry {
 private:
    NPCRegistry() = default;
    std::map<std::string, NPCTemplate> templates;

 public:
    static NPCRegistry& get_instance();

    NPCRegistry(const NPCRegistry&) = delete;
    NPCRegistry& operator=(const NPCRegistry&) = delete;

    void add_template(const std::string& id, NPCTemplate&& npc_template);
    const NPCTemplate* get_template(const std::string& id) const;
    std::vector<const NPCTemplate*> get_all_templates() const;
};

#endif
