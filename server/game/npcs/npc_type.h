#ifndef SERVER_GAME_NPCS_NPC_TYPE_SHIM_H
#define SERVER_GAME_NPCS_NPC_TYPE_SHIM_H

// El enum NPCType vive ahora en common/npc_type.h para que sea compartido
// con el cliente (deserializa el NPCSnapshot.npc_type). Este header es un
// shim que se mantiene por compatibilidad con includes viejos.
#include "common/npc_type.h"

#endif
