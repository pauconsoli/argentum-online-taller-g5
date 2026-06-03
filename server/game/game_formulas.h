#ifndef GAME_FORMULAS_H
#define GAME_FORMULAS_H

#include <memory>
#include <string>

#include "common/position.h"
#include "server/game/player_class.h"
#include "server/game/player_race.h"

class Player;

struct BaseStats {
    int strength;
    int agility;
    int intelligence;
    int constitution;
};

class GameFormulas {
 public:
    static int calculate_max_hp(const Player& player);

    static int calculate_max_mana(const Player& player);

    static int calculate_max_gold(const Player& player);

    static BaseStats calculate_base_stats(PlayerRace race, PlayerClass klass);

    static std::unique_ptr<Player> create_initial_player(uint32_t id, const std::string& name,
                                                         PlayerRace race, PlayerClass klass,
                                                         const Position& pos);
};

#endif
