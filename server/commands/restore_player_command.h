#ifndef RESTORE_PLAYER_COMMAND_H
#define RESTORE_PLAYER_COMMAND_H

#include <cstdint>
#include <memory>
#include <vector>

#include "common/commands/client_command.h"
#include "server/persistence/player_save.h"

class RestorePlayerCommand: public ClientCommand {
 private:
    uint32_t player_id;
    PlayerSave save;

 public:
    RestorePlayerCommand(uint32_t player_id, PlayerSave save):
        player_id(player_id), save(std::move(save)) {}

    uint32_t get_player_id() const override {
        return player_id;
    }

    std::vector<std::unique_ptr<GameUpdate>> execute(World& world) override;
};

#endif
