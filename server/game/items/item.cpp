#include "server/game/items/item.h"

Item::Item(const std::string& name):
        name(name) {}

Item::~Item() = default;

std::string Item::get_name() const {
    return name;
}