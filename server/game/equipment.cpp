#include "server/game/equipment.h"

#include "server/game/inventory.h"

Equipment::Equipment(Inventory& inv): inventory(&inv) {}

Item* Equipment::get_item_from_slot(EquipmentSlot slot) const {
    return inventory->get_equipped_item(slot);
}

bool Equipment::slot_has_item(EquipmentSlot slot) const {
    return get_item_from_slot(slot) != nullptr;
}
