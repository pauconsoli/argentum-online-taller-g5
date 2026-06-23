#ifndef NPC_H
#define NPC_H

#include <string>
#include <utility>
#include <vector>

#include "common/direction.h"
#include "common/npc_interact_result.h"
#include "common/npc_interaction.h"
#include "common/npc_type.h"
#include "common/position.h"
#include "server/game/character.h"

class Bank;
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
    NPCType npc_type;
    int defense;

 public:
    NPC(uint32_t id, const std::string& name, NPCType npc_type, int level, int max_hp, int defense,
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

    bool can_enter_safe_zones() const override;

    virtual InteractResult interact(NPCInteraction type, const std::string& arg, int amount,
                                    Player& player, Bank& bank);

    const std::string& get_name() const override;

    NPCType get_type() const {
        return npc_type;
    }
};

#endif
