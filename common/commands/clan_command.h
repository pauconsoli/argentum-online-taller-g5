#ifndef CLAN_COMMAND_H
#define CLAN_COMMAND_H

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "client_command.h"
#include "common/clan/clan_action.h"

class ClanCommand: public ClientCommand {
 private:
    uint32_t player_id;
    ClanAction action;
    std::string arg;

 public:
    ClanCommand(uint32_t player_id, ClanAction action, std::string arg):
        player_id(player_id), action(action), arg(std::move(arg)) {}

    uint32_t get_player_id() const override {
        return player_id;
    }

    std::vector<std::unique_ptr<GameUpdate>> execute(World& world) override;
};

#endif
