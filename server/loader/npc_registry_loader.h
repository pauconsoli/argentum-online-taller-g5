#ifndef NPC_REGISTRY_LOADER_H
#define NPC_REGISTRY_LOADER_H

#include <string>

class NPCRegistryLoader {
 public:
    static void load(const std::string& config_path);
};

#endif
