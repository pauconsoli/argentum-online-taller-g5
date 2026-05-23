#include "server/game/equipment.h"
#include "server/game/items/item.h"

Item* Equipment::get(EquipmentSlot slot) const {
    switch (slot) {
        case EquipmentSlot::WEAPON:
            return weapon;
        case EquipmentSlot::ARMOR:
            return armor;
        case EquipmentSlot::HELMET:
            return helmet;
        case EquipmentSlot::SHIELD:
            return shield;
    }
    return nullptr;
}

Item* Equipment::set(EquipmentSlot slot, Item* item) {
    Item* old = nullptr;
    switch (slot) {
        case EquipmentSlot::WEAPON:
            old = weapon;
            weapon = item;
            break;
        case EquipmentSlot::ARMOR:
            old = armor;
            armor = item;
            break;
        case EquipmentSlot::HELMET:
            old = helmet;
            helmet = item;
            break;
        case EquipmentSlot::SHIELD:
            old = shield;
            shield = item;
            break;
    }
    return old;
}

Item* Equipment::unset(EquipmentSlot slot) {
    return set(slot, nullptr);
}

bool Equipment::has(EquipmentSlot slot) const {
    return get(slot) != nullptr;
}

bool Equipment::remove(Item* item) {
    for (auto slot : {EquipmentSlot::WEAPON, EquipmentSlot::ARMOR, 
                      EquipmentSlot::HELMET, EquipmentSlot::SHIELD}) {
        if (get(slot) == item) {
            unset(slot);
            return true;
        }
    }
    return false;
}
