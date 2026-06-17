#ifndef NPC_TYPE_H
#define NPC_TYPE_H

#include <cstdint>

// Tipo canónico de NPC. Compartido entre server y cliente.
// Viaja por el wire en el NPCSnapshot.npc_type.
// El cliente lo usa directamente para mapear a sprite.
enum class NPCType : uint32_t {
    UNKNOWN = 0,
    PRIEST = 1,
    MERCHANT = 2,
    BANKER = 3,
    GOBLIN = 10,
    GOBLIN_EXECUTIONER = 11,
    SPIDER = 12,
    SPIDER_GIANT = 13,
    SKELETON = 14,
    SKELETON_WARRIOR = 15,
    ZOMBIE = 16,
    ORC = 17,
    GOLEM = 18,
    GOLEM_IRON = 19
};

#endif
