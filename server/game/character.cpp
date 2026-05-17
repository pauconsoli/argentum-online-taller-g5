#include "character.h"

Character::Character(
        uint32_t id,
        int level,
        int max_hp,
        int max_mana,
        int strength,
        int agility,
        int intelligence,
        int constitution,
        const Position& position):
        id(id),
        level(level),
        current_hp(max_hp),
        max_hp(max_hp),
        current_mana(max_mana),
        max_mana(max_mana),
        strength(strength),
        agility(agility),
        intelligence(intelligence),
        constitution(constitution),
        position(position),
        dead(false) {}

void Character::receive_damage(int damage) {
    if (damage <= 0 || dead) {
        return;
    }

    current_hp -= damage;

    if (current_hp <= 0) {
        current_hp = 0;
        dead = true;
    }
}

void Character::heal(int amount) {
    if (amount <= 0 || dead) {
        return;
    }

    current_hp += amount;

    if (current_hp > max_hp) {
        current_hp = max_hp;
    }
}

bool Character::is_dead() const {
    return dead;
}

uint32_t Character::get_id() const {
    return id;
}

int Character::get_level() const {
    return level;
}

int Character::get_current_hp() const {
    return current_hp;
}

int Character::get_max_hp() const {
    return max_hp;
}

int Character::get_current_mana() const {
    return current_mana;
}

int Character::get_max_mana() const {
    return max_mana;
}

int Character::get_strength() const {
    return strength;
}

int Character::get_agility() const {
    return agility;
}

int Character::get_intelligence() const {
    return intelligence;
}

int Character::get_constitution() const {
    return constitution;
}

const Position& Character::get_position() const {
    return position;
}

void Character::set_position(const Position& new_position) {
    position = new_position;
}

