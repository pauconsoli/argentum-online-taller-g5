#ifndef PICK_UP_ITEM_COMMAND_H
#define PICK_UP_ITEM_COMMAND_H

#include <cstdint>
#include <memory>

#include "client_command.h"

class PickUpItemCommand: public ClientCommand {
 private:
    uint32_t player_id;

 public:
    explicit PickUpItemCommand(uint32_t player_id): player_id(player_id) {}

    uint32_t get_player_id() const override {
        return player_id;
    }

    std::unique_ptr<GameUpdate> execute(World& world) override;
};

#endif
