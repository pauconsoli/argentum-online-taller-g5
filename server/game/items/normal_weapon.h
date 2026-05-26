#ifndef NORMAL_WEAPON_H
#define NORMAL_WEAPON_H

#include "server/game/items/weapon.h"

class NormalWeapon : public Weapon {
private:
    int min_damage;
    int max_damage;

    bool ranged;

public:
    NormalWeapon(const std::string& name, int min_damage, int max_damage, bool ranged);

    ~NormalWeapon() override;

    int get_min_damage() const;
    int get_max_damage() const;

    bool is_ranged() const;
};

#endif