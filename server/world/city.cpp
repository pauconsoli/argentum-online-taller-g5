#include "city.h"

#include <utility>

City::City(std::string name, Position priest_position, Position merchant_position,
           Position banker_position):
    name(std::move(name)),
    priest_position(priest_position),
    merchant_position(merchant_position),
    banker_position(banker_position) {}

bool City::is_safe() const {
    return true;
}

bool City::can_spawn() const {
    return true;
}

const std::string& City::get_name() const {
    return name;
}

Position City::get_priest_position() const {
    return priest_position;
}

Position City::get_merchant_position() const {
    return merchant_position;
}

Position City::get_banker_position() const {
    return banker_position;
}
