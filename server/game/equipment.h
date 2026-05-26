#ifndef EQUIPMENT_H
#define EQUIPMENT_H

#include "server/game/items/equipment_slot.h"

class Item;
class Inventory;

// Equipment es un wrapper que proporciona acceso a los items equipados almacenados en el inventario
class Equipment {
 private:
    Inventory* inventory;  // @not_owned: referencia al inventario

 public:
    explicit Equipment(Inventory& inv);

    // Obtiene el item equipado en un slot, o nullptr si no hay
    Item* get_item_from_slot(EquipmentSlot slot) const;

    // Verifica si hay un item equipado en el slot
    bool slot_has_item(EquipmentSlot slot) const;
};

#endif
