#ifndef CHAT_MESSAGE_UPDATE_H
#define CHAT_MESSAGE_UPDATE_H

#include <string>
#include <utility>

#include "common/updates/game_update.h"

class ChatMessageUpdate: public GameUpdate {
 private:
    uint32_t player_id;
    std::string message;

 public:
    ChatMessageUpdate(uint32_t player_id, std::string message):
        player_id(player_id), message(std::move(message)) {}

    UpdateType get_type() const override {
        return UpdateType::SYSTEM_MSG;
    }

    const std::string& get_text() const {
        return message;
    }

    uint32_t get_target_player_id() const override {
        return player_id;
    }

    const std::string& get_message() const {
        return message;
    }
};

#endif
