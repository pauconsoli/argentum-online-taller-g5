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
 private:
    static int get_random_int(int min, int max);

 public:
    // STATS

    static int calculate_max_hp(const Player& player);
    static int calculate_max_mana(const Player& player);
    static int calculate_max_gold(const Player& player);
    static BaseStats calculate_base_stats(PlayerRace race, PlayerClass klass);

    // CÁLCULOS DE RECUPERACIÓN

    static int calculate_health_recovery(const Player& player, float seconds);
    static int calculate_time_mana_recovery(const Player& player, float seconds);
    static int calculate_meditation_mana_recovery(const Player& player, float seconds);

    // SPAWN

    static std::unique_ptr<Player> create_initial_player(uint32_t id, const std::string& name,
                                                         PlayerRace race, PlayerClass klass,
                                                         const Position& pos);

    // ATAQUE

    static int calculate_damage(const Player& attacker);
    static bool calculate_evasion(const Player& attacker, const Player& target);
    static int calculate_defense(const Player& target);
    static int calculate_attack_experience_gain(const Player& attacker, const Player& target);
    static int calculate_kill_experience_gain(const Player& target);

    static int get_hand_combat_damage(const Player& attacker);
};

#endif
