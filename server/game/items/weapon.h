#ifndef WEAPON_H
#define WEAPON_H

#include <string>

#include "server/game/items/item.h"

class Weapon: public Item {
 public:
    explicit Weapon(const std::string& name);

    virtual ~Weapon();

    std::optional<EquipmentSlot> get_slot() const override;

    void use(Player& player) override;
};

#endif
