#ifndef DUNGEON_H
#define DUNGEON_H

#include <string>
#include <utility>

#include "zone.h"

class Dungeon: public Zone {
 private:
    std::string name;

 public:
    explicit Dungeon(std::string name): name(std::move(name)) {}

    bool is_safe() const override;
    bool can_spawn() const override;

    const std::string& get_name() const override;
};
#endif

// TODO(Pau): agregar NPCs de dungeon, que tengan nivel mínimo para spawnear ahí
// (más fuertes que los normales)

// No incluyo el tesoro porque lo considero un item en el suelo en una pos del dungeon
