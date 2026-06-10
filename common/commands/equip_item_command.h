#ifndef EQUIP_ITEM_COMMAND_H
#define EQUIP_ITEM_COMMAND_H

#include <memory>
#include <vector>

#include "common/commands/client_command.h"

class GameUpdate;
class World;

class EquipItemCommand: public ClientCommand {
 private:
    uint32_t player_id;
    int slot_index;

 public:
    EquipItemCommand(uint32_t player_id, int slot_index):
        player_id(player_id), slot_index(slot_index) {}
    ~EquipItemCommand() override = default;

    uint32_t get_player_id() const override {
        return player_id;
    }
    int get_slot_index() const {
        return slot_index;
    }
    std::vector<std::unique_ptr<GameUpdate>> execute(World& world) override;
};

#endif
