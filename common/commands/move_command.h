#ifndef MOVE_COMMAND_H
#define MOVE_COMMAND_H

#include <cstdint>
#include <memory>

#include "client_command.h"
#include "direction.h"

class MoveCommand: public ClientCommand {
 private:
    uint32_t player_id;
    Direction direction;

 public:
    MoveCommand(uint32_t player_id, Direction dir): player_id(player_id), direction(dir) {}

    uint32_t get_player_id() const override {
        return player_id;
    }
    Direction get_direction() const {
        return direction;
    }

    std::unique_ptr<GameUpdate> execute(World& world) override;  // para Pau
};

#endif
