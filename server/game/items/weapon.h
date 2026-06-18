#ifndef WEAPON_H
#define WEAPON_H

#include <string>

#include "server/game/items/item.h"

class Player;

struct WeaponEffect {
    int damage = 0;
    int healing = 0;
};

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

    virtual int get_mana_cost() const;

    virtual WeaponEffect apply_effect(const Player& attacker) const;

    virtual bool is_healing() const;

    virtual bool is_magic() const;

    virtual std::string get_attack_name() const;
};

#endif
