#include "server/game/items/magic_weapon.h"

#include <utility>

#include "server/game/game_formulas.h"
#include "server/game/items/spell.h"

MagicWeapon::MagicWeapon(const std::string& name, std::unique_ptr<Spell> spell, int mana_cost):
    Weapon(name, spell->get_min_damage(), spell->get_max_damage(), true),
    spell(std::move(spell)),
    mana_cost(mana_cost) {}

const Spell& MagicWeapon::get_spell() const {
    return *spell;
}

int MagicWeapon::get_mana_cost() const {
    return mana_cost;
}

WeaponEffect MagicWeapon::apply_effect(const Player& attacker) const {
    if (is_healing()) {
        int heal = GameFormulas::calculate_healing(spell->get_min_heal(), spell->get_max_heal());
        return {0, heal};
    }
    int dmg = GameFormulas::calculate_damage(attacker);
    return {dmg, 0};
}

bool MagicWeapon::is_healing() const {
    return spell->does_heal();
}
