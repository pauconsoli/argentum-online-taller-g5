#ifndef GAME_UPDATE_H
#define GAME_UPDATE_H

#include <cstdint>
#include <memory>

enum class UpdateType : uint8_t {
    LOGIN_OK,
    MATCH_LIST,
    MATCH_CREATED,
    MATCH_JOINED,
    PLAYER_JOINED,
    PLAYER_LEFT,
    PLAYER_SPAWNED,
    WORLD_MAP,
    ERROR,

    SNAPSHOT,
    MOVED,
    STATS,
    DEATH,
    ATTACKED,
    REVIVE,
    INVENTORY,
    CLAN_RESULT,
    CLAN_REVIEW,
    CATALOG,
    NPC_INTERACT,  // resultado de interacción con NPC (compra/venta/curar/etc.)
    CHAT_MSG,      // chat de jugador → jugadores (broadcast con sender_nick)
    SYSTEM_MSG,    // mensaje del sistema dirigido a 1 jugador (NPCs, errores semánticos)
};

class GameUpdate {
 public:
    virtual UpdateType get_type() const = 0;
    virtual uint32_t get_target_player_id() const {
        return 0;
    }  // 0 = broadcast, override si es para un jugador específico
    virtual ~GameUpdate() = default;
};

#endif
