#ifndef INTERACT_NPC_COMMAND_H
#define INTERACT_NPC_COMMAND_H

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "common/commands/client_command.h"
#include "common/npc_interaction.h"

class InteractNPCCommand: public ClientCommand {
 private:
    uint32_t player_id;
    uint32_t npc_id;
    NPCInteraction type;
    std::string arg;
    int amount;

 public:
    InteractNPCCommand(uint32_t player_id, uint32_t npc_id, NPCInteraction type, std::string arg,
                       int amount):
        player_id(player_id), npc_id(npc_id), type(type), arg(std::move(arg)), amount(amount) {}

    uint32_t get_player_id() const override {
        return player_id;
    }

    std::vector<std::unique_ptr<GameUpdate>> execute(World& world) override;
};

#endif
