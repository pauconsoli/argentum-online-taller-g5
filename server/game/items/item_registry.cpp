#include "server/game/items/item_registry.h"

#include <stdexcept>
#include <utility>

#include "server/game/game_formulas.h"
#include "server/game/items/consumable_item.h"
#include "server/game/items/defensive_item.h"
#include "server/game/items/magic_weapon.h"
#include "server/game/items/spell.h"
#include "server/game/items/weapon.h"

ItemRegistry& ItemRegistry::get_instance() {
    static ItemRegistry instance;
    return instance;
}

void ItemRegistry::add_template(const std::string& name, const ItemTemplate& tpl) {
    templates[name] = tpl;
    if (tpl.type == "consumable") {
        potion_keys.push_back(name);
    } else if (tpl.type == "magic_weapon") {
        magic_weapon_keys.push_back(name);
    } else if (tpl.type == "weapon") {
        weapon_keys.push_back(name);
    } else if (tpl.type == "defensive") {
        defensive_keys.push_back(name);
    } else {
        other_keys.push_back(name);
    }
}

std::unique_ptr<Item> ItemRegistry::create_item(const std::string& name) const {
    auto it = templates.find(name);
    if (it == templates.end()) {
        throw std::runtime_error("ItemRegistry: Ítem no encontrado: " + name);
    }

    const auto& t = it->second;

    if (t.type == "weapon") {
        return std::make_unique<Weapon>(t.name, t.min_damage, t.max_damage, t.ranged);
    } else if (t.type == "defensive") {
        EquipmentSlot slot;
        if (t.slot == "armor")
            slot = EquipmentSlot::ARMOR;
        else if (t.slot == "helmet")
            slot = EquipmentSlot::HELMET;
        else if (t.slot == "shield")
            slot = EquipmentSlot::SHIELD;
        else
            throw std::runtime_error("ItemRegistry: Slot inválido para " + t.name);
        return std::make_unique<DefensiveItem>(t.name, t.min_defense, t.max_defense, slot);
    } else if (t.type == "consumable") {
        ConsumableType ctype =
            (t.consumable_type == "health") ? ConsumableType::HEALTH : ConsumableType::MANA;
        return std::make_unique<ConsumableItem>(t.name, ctype, t.restore);
    } else if (t.type == "magic_weapon") {
        auto spell = std::make_unique<Spell>(t.spell_name, t.min_damage, t.max_damage, t.min_heal,
                                             t.max_heal);
        return std::make_unique<MagicWeapon>(t.name, std::move(spell), t.mana_cost);
    }
    throw std::runtime_error("ItemRegistry: Tipo de ítem desconocido: " + t.type);
}

std::unique_ptr<Item> ItemRegistry::create_random_potion() const {
    if (potion_keys.empty())
        return nullptr;
    int idx = GameFormulas::get_random_int(0, potion_keys.size() - 1);
    return create_item(potion_keys[idx]);
}

std::unique_ptr<Item> ItemRegistry::create_random_other_item() const {
    if (other_keys.empty())
        return nullptr;
    int idx = GameFormulas::get_random_int(0, other_keys.size() - 1);
    return create_item(other_keys[idx]);
}

bool ItemRegistry::exists(const std::string& name) const {
    return templates.count(name) > 0;
}

bool ItemRegistry::is_potion(const std::string& name) const {
    auto it = templates.find(name);
    return it != templates.end() && it->second.type == "consumable";
}

bool ItemRegistry::is_magic_weapon(const std::string& name) const {
    auto it = templates.find(name);
    return it != templates.end() && it->second.type == "magic_weapon";
}

bool ItemRegistry::is_weapon(const std::string& name) const {
    auto it = templates.find(name);
    return it != templates.end() && it->second.type == "weapon";
}

bool ItemRegistry::is_defensive(const std::string& name) const {
    auto it = templates.find(name);
    return it != templates.end() && it->second.type == "defensive";
}

uint64_t ItemRegistry::get_price(const std::string& name) const {
    auto it = templates.find(name);
    if (it == templates.end()) {
        return 0;  // o excepción
    }
    return it->second.price;
}
