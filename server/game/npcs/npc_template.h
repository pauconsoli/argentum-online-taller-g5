#ifndef NPC_TEMPLATE_H
#define NPC_TEMPLATE_H

#include <string>
#include <vector>

struct NPCTemplate {
    std::string id;
    std::string name;
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
