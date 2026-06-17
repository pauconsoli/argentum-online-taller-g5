#ifndef HOSTILE_NPC_H
#define HOSTILE_NPC_H

#include <string>
#include <vector>

#include "server/game/npcs/npc.h"

class HostileNPC: public NPC {
 private:
    int agility;
    int min_damage;
    int max_damage;
    int attack_range;
    std::vector<std::string> allowed_zones;

 public:
    HostileNPC(uint32_t id, const std::string& name, NPCType npc_type, int level, int max_hp,
               int defense, int agility, int min_damage, int max_damage, int attack_range,
               const std::vector<std::string>& allowed_zones, const Position& pos);

    ~HostileNPC() override = default;

    int get_agility() const override;
    int calculate_base_damage() const override;

    NPCBehavior update(float time, const std::vector<Player*>& nearby_targets) override;

    bool is_hostile() const override {
        return true;
    }

    Loot drop_loot() override;

    int get_attack_range() const override {
        return attack_range;
    }
};

#endif
