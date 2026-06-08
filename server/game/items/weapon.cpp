#include "server/game/items/weapon.h"

#include "server/game/game_formulas.h"
#include "server/game/player.h"

Weapon::Weapon(const std::string& name, int min_damage, int max_damage, bool ranged):
    Item(name), min_damage(min_damage), max_damage(max_damage), ranged(ranged) {}

std::optional<EquipmentSlot> Weapon::get_slot() const {
    return EquipmentSlot::WEAPON;
}

void Weapon::use(Player& player) {
    player.equip(*this);
}

int Weapon::get_min_damage() const {
    return min_damage;
}

int Weapon::get_max_damage() const {
    return max_damage;
}

bool Weapon::is_ranged() const {
    return ranged;
}

int Weapon::get_mana_cost() const {
    return 0;  // por defecto no consumen maná
}

WeaponEffect Weapon::apply_effect(const Player& attacker) const {
    int dmg = GameFormulas::calculate_damage(attacker);
    return {dmg, 0};
}

bool Weapon::is_healing() const {
    return false;
}
