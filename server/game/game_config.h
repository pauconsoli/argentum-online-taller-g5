#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include "player_class.h"
#include "player_race.h"

class GameConfig {
 private:
    GameConfig() = default;

 public:
    static GameConfig& get_instance();

    float get_race_hp_multiplier(PlayerRace race) const;

    float get_race_mana_multiplier(PlayerRace race) const;

    float get_class_hp_multiplier(PlayerClass player_class) const;

    float get_class_mana_multiplier(PlayerClass player_class) const;
};

#endif
