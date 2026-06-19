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

// ver si se podría dividir los spell para que cada uno calcule su propio efecto
// o sea, el spell tendría un método aparte y aca solo se llamaría a es metodo polimorficamente
WeaponEffect MagicWeapon::apply_effect(const Player& attacker) const {
    (void) attacker;  // si necesitara usar un atributo del atacante con magia, ej inteligencia, se
                      // podría usar
    if (is_healing()) {
        int heal = GameFormulas::calculate_healing(spell->get_min_heal(), spell->get_max_heal());
        return {0, heal};
    }
    int dmg = GameFormulas::calculate_base_damage_in_range(spell->get_min_damage(),
                                                           spell->get_max_damage());
    return {dmg, 0};
}

bool MagicWeapon::is_healing() const {
    return spell->does_heal();
}

bool MagicWeapon::is_magic() const {
    return true;
}

std::string MagicWeapon::get_attack_name() const {
    return spell->get_name();
}
