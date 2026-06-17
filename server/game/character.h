#ifndef CHARACTER_H
#define CHARACTER_H

// Character: clase base para Player y NPC, contiene atributos comunes a ambos
// "tipos" de personajes, como HP, mana, fuerza, etc. También tiene métodos
// para recibir daño, curarse, etc

#include <cstdint>
#include <string>
#include <utility>

#include "common/attack_result.h"
#include "common/position.h"
#include "server/game/loot.h"

class Character {
 protected:
    uint32_t id;

    int level;

    int current_hp;
    int max_hp;

    Position position;

    bool dead;

 public:
    Character(uint32_t id, int level, int max_hp, const Position& position);

    virtual ~Character() = default;

    virtual bool can_cast_magic() const = 0;

    virtual int get_defense()
        const = 0;  // para NPCs que no tengan inventario valor fijo, para Players se calcula con el
                    // inventario equipado usando la formula de GameFormulas

    virtual bool validate_attack_from(int attacker_level) const = 0;

    virtual int get_agility() const = 0;

    virtual Loot drop_loot() = 0;

    virtual void add_experience(uint64_t /*amount*/) {}

    virtual bool is_healing_attack() const {
        return false;
    }
    virtual bool is_ranged_attack() const {
        return false;
    }
    virtual bool is_magic_attack() const {
        return false;
    }
    virtual std::string get_attack_name() const {
        return "";
    }
    virtual int calculate_base_damage() const = 0;
    virtual int calculate_base_healing() const {
        return 0;
    }
    virtual AttackStatus consume_attack_resources() {
        return AttackStatus::SUCCESS;
    }

    void receive_damage(int damage);

    void heal(int amount);

    bool is_dead() const;

    uint32_t get_id() const;

    int get_level() const;

    int get_current_hp() const;
    int get_max_hp() const;

    const Position& get_position() const;

    void set_position(const Position& new_position);
    void set_level(int new_level);
};

#endif
