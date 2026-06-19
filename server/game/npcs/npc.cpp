#include "server/game/npcs/npc.h"

#include "common/npc_interact_result.h"

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

bool NPC::can_enter_safe_zones() const {
    return false;
}

InteractResult NPC::interact(NPCInteraction /*type*/, const std::string& /*arg*/, int /*amount*/,
                             Player& /*player*/, Bank& /*bank*/) {
    // por defecto no interactuan, y los de ciudad lo overridean
    return InteractResult{InteractStatus::NOT_ALLOWED};
}

const std::string& NPC::get_name() const {
    return name;
}
