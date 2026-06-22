#ifndef ZONE_TYPE_H
#define ZONE_TYPE_H

#include <string>

enum class ZoneType {
    ALL,
    CITY,
    DUNGEON,
};

// si rse agregaran más zonas, se agregan acá y se parsean en el .cpp
ZoneType parse_zone_type(const std::string& type);

#endif
