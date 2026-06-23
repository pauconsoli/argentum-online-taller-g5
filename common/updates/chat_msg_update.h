#ifndef CHAT_MSG_UPDATE_H
#define CHAT_MSG_UPDATE_H

#include <cstdint>
#include <string>
#include <utility>

#include "game_update.h"

// Mensajes de jugadores

class ChatMsgUpdate: public GameUpdate {
 public:
    uint32_t sender_id;
    std::string sender_nick;
    std::string text;
    uint32_t recipient_id;  // 0 = broadcast; non-zero = sólo ese jugador (routing server-side)
    bool is_private;        // true = mostrar como mensaje privado en el cliente

    ChatMsgUpdate(uint32_t sender_id, std::string sender_nick, std::string text,
                  uint32_t recipient_id = 0, bool is_private = false):
        sender_id(sender_id),
        sender_nick(std::move(sender_nick)),
        text(std::move(text)),
        recipient_id(recipient_id),
        is_private(is_private) {}

    UpdateType get_type() const override {
        return UpdateType::CHAT_MSG;
    }

    uint32_t get_target_player_id() const override {
        return recipient_id;
    }
};

#endif
