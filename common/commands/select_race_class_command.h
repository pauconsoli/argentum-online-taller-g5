#ifndef SELECT_RACE_CLASS_COMMAND_H
#define SELECT_RACE_CLASS_COMMAND_H

#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#include "client_command.h"
#include "server/game/player_class.h"
#include "server/game/player_race.h"

class SelectRaceClassCommand: public ClientCommand {
 private:
    uint32_t player_id;
    std::string nick;
    PlayerRace race;
    PlayerClass klass;

 public:
    SelectRaceClassCommand(uint32_t player_id, std::string nick, PlayerRace race,
                           PlayerClass klass):
        player_id(player_id), nick(std::move(nick)), race(race), klass(klass) {}

    uint32_t get_player_id() const override {
        return player_id;
    }

    std::unique_ptr<GameUpdate> execute(World& world) override;
};

#endif
