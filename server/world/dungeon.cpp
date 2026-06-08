#include "dungeon.h"

#include <string>

Dungeon::Dungeon(std::string name, int entrance_x, int entrance_y):
    name(std::move(name)), entrance_x(entrance_x), entrance_y(entrance_y) {}

bool Dungeon::is_safe() const {
    return false;
}

bool Dungeon::can_spawn() const {
    return true;
}

const std::string& Dungeon::get_name() const {
    return name;
}
