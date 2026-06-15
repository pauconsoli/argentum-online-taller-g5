#ifndef ITEM_REGISTRY_H
#define ITEM_REGISTRY_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

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
        uint64_t price = 0;
    };

 private:
    std::unordered_map<std::string, ItemTemplate> templates;
    std::vector<std::string> potion_keys;
    std::vector<std::string> magic_weapon_keys;
    std::vector<std::string> weapon_keys;
    std::vector<std::string> defensive_keys;
    std::vector<std::string>
        other_keys;  // para cualquier otro tipo de ítem que pueda haber en el futuro

    ItemRegistry() = default;

 public:
    static ItemRegistry& get_instance();

    ItemRegistry(const ItemRegistry&) = delete;
    ItemRegistry& operator=(const ItemRegistry&) = delete;

    void add_template(const std::string& name, const ItemTemplate& tpl);

    bool exists(const std::string& name) const;
    bool is_potion(const std::string& name) const;
    bool is_magic_weapon(const std::string& name) const;
    bool is_weapon(const std::string& name) const;
    bool is_defensive(const std::string& name) const;
    uint64_t get_price(const std::string& name) const;

    const std::vector<std::string>& get_potion_keys() const {
        return potion_keys;
    }
    const std::vector<std::string>& get_other_keys() const {
        return other_keys;
    }
    const std::vector<std::string>& get_magic_weapon_keys() const {
        return magic_weapon_keys;
    }
    const std::vector<std::string>& get_weapon_keys() const {
        return weapon_keys;
    }
    const std::vector<std::string>& get_defensive_keys() const {
        return defensive_keys;
    }

    std::unique_ptr<Item> create_item(const std::string& name) const;
    std::unique_ptr<Item> create_random_potion() const;
    std::unique_ptr<Item> create_random_other_item() const;
};

#endif
