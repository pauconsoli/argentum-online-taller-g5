#ifndef DROP_ITEM_COMMAND_H
#define DROP_ITEM_COMMAND_H

#include <cstdint>
#include <memory>
#include <vector>

#include "client_command.h"

class DropItemCommand: public ClientCommand {
 private:
    uint32_t player_id;
    int slot_index;

 public:
    DropItemCommand(uint32_t player_id, int slot_index):
        player_id(player_id), slot_index(slot_index) {}

    uint32_t get_player_id() const override {
        return player_id;
    }
    int get_slot_index() const {
        return slot_index;
    }

    std::vector<std::unique_ptr<GameUpdate>> execute(World& world) override;
};

#endif
