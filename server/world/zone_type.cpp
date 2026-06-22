#include "zone_type.h"

#include <stdexcept>

ZoneType parse_zone_type(const std::string& type) {
    if (type == "all")
        return ZoneType::ALL;
    if (type == "city")
        return ZoneType::CITY;
    if (type == "dungeon")
        return ZoneType::DUNGEON;
    throw std::runtime_error("Unknown zone type: " + type);
}
