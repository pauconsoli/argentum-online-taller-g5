#ifndef NPC_TYPE_H
#define NPC_TYPE_H

#include <cstdint>

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
