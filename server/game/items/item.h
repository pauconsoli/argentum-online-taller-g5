#ifndef ITEM_H
#define ITEM_H

#include <optional>
#include <string>

#include "server/game/items/equipment_slot.h"

class Player;  // o character? no recuerdo si los npcs pueden tener equipamiento

class Item {
 protected:
    std::string name;

 public:
    explicit Item(const std::string& name);
    virtual ~Item();

    std::string get_name() const;

    virtual void use(Player& player) = 0;  // consumir, equipar, etc

    virtual std::optional<EquipmentSlot> get_slot()
        const = 0;  // no se si es lo mejor. nullopt si no es equipable
};

#endif
