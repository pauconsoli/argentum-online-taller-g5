#ifndef ITEM_H
#define ITEM_H

#include <optional>
#include <string>

#include "server/game/items/equipment_slot.h"

class Player;

class Item {
 protected:
    std::string name;

 public:
    explicit Item(const std::string& name);
    virtual ~Item();

    std::string get_name() const;

    virtual void use(Player& player) = 0;  // consumir, equipar, etc

    virtual std::optional<EquipmentSlot> get_slot() const = 0;
};

#endif
