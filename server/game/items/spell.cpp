#include "server/game/items/spell.h"

Spell::Spell(const std::string& name, int min_damage, int max_damage, int min_heal, int max_heal):
    name(name),
    min_damage(min_damage),
    max_damage(max_damage),
    min_heal(min_heal),
    max_heal(max_heal) {}

Spell::~Spell() = default;

std::string Spell::get_name() const {
    return name;
}

int Spell::get_min_damage() const {
    return min_damage;
}

int Spell::get_max_damage() const {
    return max_damage;
}

int Spell::get_min_heal() const {
    return min_heal;
}

int Spell::get_max_heal() const {
    return max_heal;
}

bool Spell::does_damage() const {
    return max_damage > 0;
}

bool Spell::does_heal() const {
    return max_heal > 0;
}
