#ifndef MAGIC_WEAPON_H
#define MAGIC_WEAPON_H

#include <memory>
#include <string>

#include "server/game/items/weapon.h"

class Spell;

class MagicWeapon: public Weapon {
 private:
    std::unique_ptr<Spell> spell;

    int mana_cost;

 public:
    MagicWeapon(const std::string& name, std::unique_ptr<Spell> spell, int mana_cost);

    ~MagicWeapon() override;

    const Spell& get_spell() const;

    int get_mana_cost() const;
};

#endif
