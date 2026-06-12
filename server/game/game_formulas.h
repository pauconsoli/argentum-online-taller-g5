#ifndef GAME_FORMULAS_H
#define GAME_FORMULAS_H

#include <memory>
#include <string>

#include "common/position.h"
#include "server/game/character.h"
#include "server/game/player_class.h"
#include "server/game/player_race.h"

class Player;
class Weapon;

struct BaseStats {
    int strength;
    int agility;
    int intelligence;
    int constitution;
};

class GameFormulas {
 private:
    static int get_random_int(int min, int max);
    static float get_random_float(float min, float max);

 public:
    // STATS

    static int calculate_max_hp(const Player& player);
    static int calculate_max_mana(const Player& player);
    static int calculate_max_gold(const Player& player);
    static uint64_t calculate_level_up_limit(int level);
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

    static bool can_attack_by_level(int attacker_level, int target_level);
    static int calculate_damage(const Player& attacker, const Weapon* weapon = nullptr);
    static bool calculate_critical_attack();
    static bool calculate_evasion(const Character& target);
    static int calculate_defense(const Player& target);
    static int calculate_healing(int min_heal, int max_heal);
    static int calculate_attack_experience_gain(const Character& attacker, const Character& target,
                                                int damage);
    static int calculate_kill_experience_gain(const Character& attacker, const Character& target);

    static int get_hand_combat_damage(const Player& attacker);

    // MUERTE Y DROPS
    static uint64_t calculate_player_dropped_gold(const Player& player);
    static uint64_t calculate_player_dropped_experience(const Player& player);
    static uint64_t calculate_npc_dropped_gold(const Character& npc);
};

#endif
