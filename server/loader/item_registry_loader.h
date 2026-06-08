#ifndef ITEM_REGISTRY_LOADER_H
#define ITEM_REGISTRY_LOADER_H

#include <string>

class ItemRegistryLoader {
 public:
    static void load(const std::string& config_path);
};

#endif
