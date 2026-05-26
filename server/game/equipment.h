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

    Item* equip(Item* item, EquipmentSlot slot); // devuelve el item que estaba equipado o nullptr si el slot estaba vacío
    Item* unequip(EquipmentSlot slot); // devuelve el item desequipado

    Item* get(EquipmentSlot slot) const;
    bool has(EquipmentSlot slot) const;
};

#endif