#ifndef ITEM_REGISTRY_H
#define ITEM_REGISTRY_H

#include <memory>
#include <string>
#include <unordered_map>

#include "server/game/items/item.h"

class ItemRegistry {
 public:
    struct ItemTemplate {
        std::string name;
        std::string type;
        int min_damage = 0;
        int max_damage = 0;
        bool ranged = false;
        int min_defense = 0;
        int max_defense = 0;
        std::string slot;
        std::string consumable_type;
        int restore = 0;
        std::string spell_name;
        int min_heal = 0;
        int max_heal = 0;
        int mana_cost = 0;
    };

 private:
    std::unordered_map<std::string, ItemTemplate> templates;

    ItemRegistry() = default;

 public:
    static ItemRegistry& get_instance();

    ItemRegistry(const ItemRegistry&) = delete;
    ItemRegistry& operator=(const ItemRegistry&) = delete;

    void add_template(const std::string& name, const ItemTemplate& tpl);

    std::unique_ptr<Item> create_item(const std::string& name) const;
};

#endif
