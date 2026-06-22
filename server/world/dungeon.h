#ifndef DUNGEON_H
#define DUNGEON_H

#include <string>
#include <utility>

#include "zone.h"

class Dungeon: public Zone {
 private:
    std::string name;
    int entrance_x;
    int entrance_y;

 public:
    Dungeon(std::string name, int entrance_x, int entrance_y);

    bool is_safe() const override;
    bool can_spawn() const override;

    const std::string& get_name() const override;
    ZoneType get_type() const override;

    int get_entrance_x() const;
    int get_entrance_y() const;
};
#endif
