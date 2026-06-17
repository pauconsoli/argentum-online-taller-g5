#ifndef NPC_TEMPLATE_H
#define NPC_TEMPLATE_H

#include <string>
#include <vector>

#include "server/game/npcs/npc_type.h"

struct NPCTemplate {
    std::string id;
    std::string name;
    NPCType npc_type;
    int level;
    int max_hp;
    int defense;
    int agility;
    int min_damage;
    int max_damage;
    int attack_range;
    std::vector<std::string> zones;
};

#endif
