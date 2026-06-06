#ifndef CHARACTER_H
#define CHARACTER_H

// Character: clase base para Player y NPC, contiene atributos comunes a ambos
// "tipos" de personajes, como HP, mana, fuerza, etc. También tiene métodos
// para recibir daño, curarse, etc

#include <cstdint>
#include <string>
#include <utility>

#include "common/position.h"

class Character {
 protected:
    uint32_t id;

    int level;

    int current_hp;
    int max_hp;

    int current_mana;
    int max_mana;

    int strength;
    int agility;
    int intelligence;
    int constitution;

    Position position;

    bool dead;

 public:
    Character(uint32_t id, int level, int max_hp, int max_mana, int strength, int agility,
              int intelligence, int constitution, const Position& position);

    virtual ~Character() = default;

    virtual bool can_cast_magic() const = 0;

    void receive_damage(int damage);

    void heal(int amount);

    bool is_dead() const;

    uint32_t get_id() const;

    int get_level() const;  // npc tiene level? lo dejo por las dudas

    int get_current_hp() const;
    int get_max_hp() const;

    int get_current_mana() const;
    int get_max_mana() const;

    int get_strength() const;
    int get_agility() const;
    int get_intelligence() const;
    int get_constitution() const;

    const Position& get_position() const;

    void set_position(const Position& new_position);
    void set_level(int new_level);
};

#endif
