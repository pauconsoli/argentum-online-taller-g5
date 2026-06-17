#ifndef NPC_VISUAL_TYPE_H
#define NPC_VISUAL_TYPE_H

#include <cstdint>

// Tipo visual de NPC, exclusivo del cliente SDL.
// Cuando Reni agregue el tipo compartido del protocolo, se agregará un adaptador
// NPCSpriteType → NPCVisualType en el handler de SnapshotUpdate.
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

#endif
