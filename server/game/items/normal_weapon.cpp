#include "server/game/items/normal_weapon.h"

NormalWeapon::NormalWeapon(const std::string& name, int min_damage, int max_damage, bool ranged):
    Weapon(name), min_damage(min_damage), max_damage(max_damage), ranged(ranged) {}

NormalWeapon::~NormalWeapon() = default;

int NormalWeapon::get_min_damage() const {
    return min_damage;
}

int NormalWeapon::get_max_damage() const {
    return max_damage;
}

bool NormalWeapon::is_ranged() const {
    return ranged;
}
