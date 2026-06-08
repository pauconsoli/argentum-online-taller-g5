#include "city.h"

#include <utility>

City::City(std::string name): name(std::move(name)) {}

bool City::is_safe() const {
    return true;
}

bool City::can_spawn() const {
    return true;
}

const std::string& City::get_name() const {
    return name;
}
