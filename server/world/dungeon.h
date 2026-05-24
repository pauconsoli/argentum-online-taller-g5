#ifndef DUNGEON_H
#define DUNGEON_H

#include "zone.h"

class Dungeon : public Zone {
public:
    bool is_safe() const override;

    bool can_spawn() const override;
};

#endif