#ifndef NPC_VISUAL_TYPE_H
#define NPC_VISUAL_TYPE_H

#include <cstdint>

enum class NPCVisualType : uint8_t {
    UNKNOWN = 0,
    BANKER = 1,
    PRIEST = 2,
    MERCHANT = 3,
    GOBLIN = 4,
    SKELETON = 5,
    ZOMBIE = 6,
    SPIDER = 7,
    ORC = 8,
    GOLEM_ICE = 9,
    GOLEM_STONE = 10,
    GOLEM_INFERNAL = 11,
};

inline NPCVisualType npc_visual_type_from_network(uint32_t npc_type) {
    switch (npc_type) {
        case 1:
            return NPCVisualType::PRIEST;
        case 2:
            return NPCVisualType::MERCHANT;
        case 3:
            return NPCVisualType::BANKER;
        case 9:
            return NPCVisualType::GOLEM_INFERNAL;
        case 10:
            return NPCVisualType::GOBLIN;
        case 11:
            return NPCVisualType::GOBLIN;
        case 12:
            return NPCVisualType::SPIDER;
        case 13:
            return NPCVisualType::SPIDER;
        case 14:
            return NPCVisualType::SKELETON;
        case 15:
            return NPCVisualType::SKELETON;
        case 16:
            return NPCVisualType::ZOMBIE;
        case 17:
            return NPCVisualType::ORC;
        case 18:
            return NPCVisualType::GOLEM_STONE;
        case 19:
            return NPCVisualType::GOLEM_ICE;
        default:
            return NPCVisualType::UNKNOWN;
    }
}

#endif
