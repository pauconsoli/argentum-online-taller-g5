#include "server/game/npcs/npc.h"

NPC::NPC(uint32_t id, const std::string& name, int level, int max_hp, int defense,
         const Position& pos):
    Character(id, level, max_hp, pos), name(name), defense(defense) {}

int NPC::get_defense() const {
    return defense;
}

int NPC::get_agility() const {
    return 0;  // por defecto
}

const std::string& NPC::get_name() const {
    return name;
}
