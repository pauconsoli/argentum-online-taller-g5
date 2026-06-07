#ifndef SPAWNED_UPDATE_H
#define SPAWNED_UPDATE_H

#include <cstdint>
#include <string>

#include "common/position.h"
#include "game_update.h"

class SpawnedUpdate: public GameUpdate {
 private:
    uint32_t player_id;
    std::string nick;
    PlayerRace race;
    PlayerClass klass;
    Position pos;

 public:
    SpawnedUpdate(uint32_t player_id, std::string nick, PlayerRace race, PlayerClass klass,
                  Position pos):
        player_id(player_id), nick(nick), race(race), klass(klass), pos(pos) {}

    UpdateType get_type() const override {
        return UpdateType::PLAYER_SPAWNED;
    }

    uint32_t get_player_id() const {
        return player_id;
    }
    const std::string& get_nick() const {
        return nick;
    }
    PlayerRace get_race() const {
        return race;
    }
    PlayerClass get_class() const {
        return klass;
    }
    Position get_pos() const {
        return pos;
    }
};

#endif
