#include "server/game/equipment.h"

#include "server/game/items/item.h"

Equipment::Equipment() {
    equipment[EquipmentSlot::WEAPON] = nullptr;
    equipment[EquipmentSlot::ARMOR] = nullptr;
    equipment[EquipmentSlot::HELMET] = nullptr;
    equipment[EquipmentSlot::SHIELD] = nullptr;
}

Item* Equipment::get_item_from_slot(EquipmentSlot slot) const {
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

bool Equipment::slot_has_item(EquipmentSlot slot) const {
    return get_item_from_slot(slot) != nullptr;
}
