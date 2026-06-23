#ifndef CHARACTER_H
#define CHARACTER_H

#include <cstdint>
#include <string>
#include <utility>

#include "common/attack_result.h"
#include "common/direction.h"
#include "common/position.h"
#include "server/game/loot.h"
#include "server/world/zone_type.h"

class Character {
 protected:
    uint32_t id;

    int level;

    int current_hp;
    int max_hp;

    Position position;

    bool dead;

    Direction direction;
    bool moving;

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

    virtual const std::string& get_name() const = 0;

    virtual bool can_enter_safe_zones() const = 0;

    virtual bool can_enter_zone_type(ZoneType /*zone_type*/) const {
        return true;
    }

    void receive_damage(int damage);

    int heal(int amount);

    bool is_dead() const;

    uint32_t get_id() const;

    int get_level() const;

    int get_current_hp() const;
    int get_max_hp() const;

    virtual uint32_t get_clan_id() const {
        return 0;  // por defecto no tienen clan (npcs)
    }

    const Position& get_position() const;

    void set_position(const Position& new_position);
    void set_level(int new_level);

    Direction get_direction() const {
        return direction;
    }
    void set_direction(Direction dir) {
        direction = dir;
    }

    bool is_moving() const {
        return moving;
    }
    void set_moving(bool is_moving) {
        moving = is_moving;
    }
};

#endif
