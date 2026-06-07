#ifndef MEDITATE_COMMAND_H
#define MEDITATE_COMMAND_H

#include <cstdint>
#include <memory>
#include <vector>

#include "client_command.h"

class MeditateCommand: public ClientCommand {
 private:
    uint32_t player_id;

 public:
    explicit MeditateCommand(uint32_t player_id): player_id(player_id) {}

    uint32_t get_player_id() const override {
        return player_id;
    }

    std::vector<std::unique_ptr<GameUpdate>> execute(World& world) override;
};

#endif
