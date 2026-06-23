#include "character.h"

Character::Character(uint32_t id, int level, int max_hp, const Position& position):
    id(id),
    level(level),
    current_hp(max_hp),
    max_hp(max_hp),
    position(position),
    dead(false),
    direction(Direction::DOWN),
    moving(false) {}

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

int Character::heal(int amount) {
    if (amount <= 0 || dead) {
        return 0;
    }
    int old_hp = current_hp;
    current_hp += amount;

    if (current_hp > max_hp) {
        current_hp = max_hp;
    }
    return current_hp - old_hp;
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

const Position& Character::get_position() const {
    return position;
}

void Character::set_position(const Position& new_position) {
    position = new_position;
}

void Character::set_level(int new_level) {
    level = new_level;
}
