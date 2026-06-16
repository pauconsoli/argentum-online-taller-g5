#ifndef NPC_H
#define NPC_H

#include <string>
#include <vector>

#include "common/position.h"
#include "server/game/character.h"

class Player;

enum class NPCAction { STILL, CHASE, ATTACK };

struct NPCBehavior {
    NPCAction action = NPCAction::STILL;
    Position target_position = {0, 0};
    uint32_t target_id = 0;
};

class NPC: public Character {
 protected:
    std::string name;
    int defense;

 public:
    NPC(uint32_t id, const std::string& name, int level, int max_hp, int defense,
        const Position& pos);

    ~NPC() override = default;

    bool can_cast_magic() const override {
        return false;
    }
    bool validate_attack_from(int /*attacker_level*/) const override {
        return true;
    }
    int get_defense() const override;
    int get_agility() const override;
    int calculate_base_damage() const override;

    virtual NPCBehavior update(float time, const std::vector<Player*>& nearby_targets) = 0;
    virtual bool is_hostile() const = 0;

    virtual int get_attack_range() const {
        return 0;
    }

    const std::string& get_name() const override;
};

#endif
