#ifndef EQUIPMENT_H
#define EQUIPMENT_H

#include "server/game/items/equipment_slot.h"

class Item;

// Equipment: gestiona los 4 slots de equipamiento del jugador
// - Un item por cada slot
// - nullptr si el slot está vacío
class Equipment {
private:
    Item* weapon = nullptr;
    Item* armor = nullptr;
    Item* helmet = nullptr;
    Item* shield = nullptr;

public:
    Equipment() = default;
    ~Equipment() = default;

    Equipment(const Equipment&) = delete;
    Equipment& operator=(const Equipment&) = delete;

    // Obtener el item equipado en un slot
    Item* get(EquipmentSlot slot) const;

    // Equipar un item en su slot (devuelve lo que había, nullptr si nada)
    Item* set(EquipmentSlot slot, Item* item);

    // Desequipar un slot
    Item* unset(EquipmentSlot slot);

    // Verificar si un slot tiene algo equipado
    bool has(EquipmentSlot slot) const;

    // Remover item de cualquier slot donde esté (usado en remove_item)
    bool remove(Item* item);
};

#endif
