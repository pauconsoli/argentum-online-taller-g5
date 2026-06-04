#include "server/game/items/magic_weapon.h"

#include <utility>

#include "server/game/items/spell.h"

MagicWeapon::MagicWeapon(const std::string& name, std::unique_ptr<Spell> spell, int mana_cost):
    Weapon(name), spell(std::move(spell)), mana_cost(mana_cost) {}

MagicWeapon::~MagicWeapon() = default;

const Spell& MagicWeapon::get_spell() const {
    return *spell;
}

int MagicWeapon::get_mana_cost() const {
    return mana_cost;
}
