#include "server/game/npcs/npc.h"

NPC::NPC(uint32_t id, const std::string& name, NPCType npc_type, int level, int max_hp, int defense,
         const Position& pos):
    Character(id, level, max_hp, pos),
    name(name),
    npc_type(npc_type),
    defense(defense),
    direction(Direction::DOWN),
    moving(false) {}

int NPC::get_defense() const {
    return defense;
}

int NPC::get_agility() const {
    return 0;  // por defecto
}

int NPC::calculate_base_damage() const {
    return 0;
}

const std::string& NPC::get_name() const {
    return name;
}
