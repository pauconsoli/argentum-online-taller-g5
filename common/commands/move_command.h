#ifndef MOVE_COMMAND_H
#define MOVE_COMMAND_H

#include <cstdint>
#include "client_command.h"

enum class Direction : uint8_t {
    UP    = 0x01,
    DOWN  = 0x02,
    LEFT  = 0x03,
    RIGHT = 0x04,
};

class MoveCommand : public ClientCommand {
private:
    uint16_t player_id;
    Direction direction;

public:
    MoveCommand(uint16_t player_id, Direction dir)
        : player_id(player_id), direction(dir) {}

    uint16_t get_player_id() const { return player_id; }
    Direction get_direction() const { return direction; }

    void execute(World& world) override;  // para Pau 
};

#endif