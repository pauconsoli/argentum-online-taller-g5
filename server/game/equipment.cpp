#include "server/game/equipment.h"
#include "server/game/items/item.h"

Equipment::Equipment() {
    equipment[EquipmentSlot::WEAPON] = nullptr;
    equipment[EquipmentSlot::ARMOR] = nullptr;
    equipment[EquipmentSlot::HELMET] = nullptr;
    equipment[EquipmentSlot::SHIELD] = nullptr;
}

Item* Equipment::get(EquipmentSlot slot) const {
    auto it = equipment.find(slot);
    if (it != equipment.end()) {
        return it->second;
    }
    return nullptr;
}

Item* Equipment::equip(Item* item, EquipmentSlot slot) {
    Item* prev = equipment[slot];  // nullptr si no existía
    equipment[slot] = item;
    return prev;
}

Item* Equipment::unequip(EquipmentSlot slot) {
    Item* item = equipment[slot];
    equipment[slot] = nullptr;
    return item;  // nullptr si el slot estaba vacío
}

bool Equipment::has(EquipmentSlot slot) const {
    return get(slot) != nullptr;
}