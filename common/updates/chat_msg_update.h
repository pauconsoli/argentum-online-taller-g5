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

    ChatMsgUpdate(uint32_t sender_id, std::string sender_nick, std::string text):
        sender_id(sender_id), sender_nick(std::move(sender_nick)), text(std::move(text)) {}

    UpdateType get_type() const override {
        return UpdateType::CHAT_MSG;
    }
};

#endif
