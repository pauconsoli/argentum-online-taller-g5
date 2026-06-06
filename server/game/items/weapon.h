#ifndef WEAPON_H
#define WEAPON_H

#include <string>

#include "server/game/items/item.h"

class Weapon: public Item {
 protected:
    int min_damage;
    int max_damage;
    bool ranged;

 public:
    Weapon(const std::string& name, int min_damage, int max_damage, bool ranged);

    virtual ~Weapon() = default;

    std::optional<EquipmentSlot> get_slot() const override;

    void use(Player& player) override;

    int get_min_damage() const;
    int get_max_damage() const;

    virtual bool is_ranged() const;
};

#endif
