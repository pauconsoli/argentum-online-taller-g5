#ifndef CHAT_COMMAND_H
#define CHAT_COMMAND_H

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "client_command.h"

class ChatCommand: public ClientCommand {
 private:
    uint32_t player_id;
    std::string nick;
    std::string text;

 public:
    ChatCommand(uint32_t player_id, std::string nick, std::string text):
        player_id(player_id), nick(std::move(nick)), text(std::move(text)) {}

    uint32_t get_player_id() const override {
        return player_id;
    }

    std::vector<std::unique_ptr<GameUpdate>> execute(World& world) override;
};

#endif
