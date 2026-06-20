#ifndef CHEAT_COMMAND_H
#define CHEAT_COMMAND_H

#include <cstdint>
#include <memory>
#include <vector>

#include "client_command.h"

enum class CheatType : uint8_t;

class CheatCommand: public ClientCommand {
 private:
    uint32_t player_id;
    CheatType cheat_type;

 public:
    CheatCommand(uint32_t player_id, CheatType cheat_type):
        player_id(player_id), cheat_type(cheat_type) {}

    uint32_t get_player_id() const override {
        return player_id;
    }

    std::vector<std::unique_ptr<GameUpdate>> execute(World& world) override;
};

#endif
