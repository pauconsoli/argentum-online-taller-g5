#include "dungeon.h"

#include <string>

bool Dungeon::is_safe() const {
    return false;
}

bool Dungeon::can_spawn() const {
    return true;
}

const std::string& Dungeon::get_name() const {
    return name;
}
