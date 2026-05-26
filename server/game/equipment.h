#ifndef EQUIPMENT_H
#define EQUIPMENT_H

#include <map>

#include "server/game/items/equipment_slot.h"

class Item;

class Equipment {
 private:
    std::map<EquipmentSlot, Item*> equipment;

 public:
    Equipment();

    Item* equip(Item* item, EquipmentSlot slot);  // devuelve el item que estaba equipado o nullptr
                                                  // si el slot estaba vacío
    Item* unequip(EquipmentSlot slot);            // devuelve el item desequipado

    Item* get_item_from_slot(EquipmentSlot slot) const;
    bool slot_has_item(EquipmentSlot slot) const;
};

#endif
