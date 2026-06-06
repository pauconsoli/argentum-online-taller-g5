#ifndef ATTACK_COMMAND_H
#define ATTACK_COMMAND_H

#include <cstdint>
#include <memory>

#include "client_command.h"

class AttackCommand: public ClientCommand {
 private:
    uint32_t player_id;
    uint32_t target_id;

 public:
    AttackCommand(uint32_t player_id, uint32_t target_id):
        player_id(player_id), target_id(target_id) {}

    uint32_t get_player_id() const override {
        return player_id;
    }
    uint32_t get_target_id() const {
        return target_id;
    }

    std::unique_ptr<GameUpdate> execute(World& world) override;
};

#endif
