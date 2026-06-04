#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include <map>

#include "common/position.h"
#include "player_class.h"
#include "player_race.h"

// hay que inicializar única vez al arrancar el servidor GameConfigLoader::load(path)

class GameConfig {

 private:
    GameConfig();

    friend class GameConfigLoader;

    bool loaded = false;  // true al terminar GameConfigLoader::load()

    std::map<PlayerRace, float> race_hp_mult;
    std::map<PlayerRace, float> race_mana_mult;
    std::map<PlayerRace, float> race_recovery;

    std::map<PlayerRace, int> race_base_strength;
    std::map<PlayerRace, int> race_base_agility;
    std::map<PlayerRace, int> race_base_intelligence;
    std::map<PlayerRace, int> race_base_constitution;

    std::map<PlayerClass, int> class_bonus_strength;
    std::map<PlayerClass, int> class_bonus_agility;
    std::map<PlayerClass, int> class_bonus_intelligence;
    std::map<PlayerClass, int> class_bonus_constitution;

    std::map<PlayerClass, float> class_hp_mult;
    std::map<PlayerClass, float> class_mana_mult;
    std::map<PlayerClass, float> class_meditation;

    int max_inventory_items;

    Position spawn_position;

    int initial_player_level;

    float gold_max_safe_base;
    float gold_max_safe_exp;
    float gold_excess_factor;
    float npc_gold_drop_factor;

    float level_limit_base;
    float level_limit_exp;
    float kill_bonus_factor;

    float critical_chance;
    float dodge_threshold;
    int newbie_max_level;
    int max_level_difference;

    int clan_max_members;
    int clan_min_level_to_found;

    float npc_drop_nothing_prob;
    float npc_drop_gold_prob;
    float npc_drop_potion_prob;
    float npc_drop_item_prob;
    float npc_gold_drop_min_factor;
    float npc_gold_drop_max_factor;

    int server_game_loop_sleep_ms;

 public:
    static GameConfig& get_instance();

    float get_race_hp_multiplier(PlayerRace race) const;
    float get_race_mana_multiplier(PlayerRace race) const;
    float get_race_recovery_factor(PlayerRace race) const;

    int get_race_strength(PlayerRace race) const;
    int get_race_agility(PlayerRace race) const;
    int get_race_intelligence(PlayerRace race) const;
    int get_race_constitution(PlayerRace race) const;

    int get_class_bonus_strength(PlayerClass cls) const;
    int get_class_bonus_agility(PlayerClass cls) const;
    int get_class_bonus_intelligence(PlayerClass cls) const;
    int get_class_bonus_constitution(PlayerClass cls) const;

    float get_class_hp_multiplier(PlayerClass cls) const;
    float get_class_mana_multiplier(PlayerClass cls) const;
    float get_class_meditation_factor(PlayerClass cls) const;

    int get_max_inventory_items() const;

    Position get_spawn_position() const;

    int get_initial_player_level() const;

    float get_gold_max_safe_base() const;
    float get_gold_max_safe_exp() const;
    float get_gold_excess_factor() const;
    float get_npc_gold_drop_factor() const;

    float get_level_limit_base() const;
    float get_level_limit_exp() const;
    float get_kill_bonus_factor() const;

    float get_critical_chance() const;
    float get_dodge_threshold() const;
    int get_newbie_max_level() const;
    int get_max_level_difference() const;

    int get_clan_max_members() const;
    int get_clan_min_level_to_found() const;

    float get_npc_drop_nothing_prob() const;
    float get_npc_drop_gold_prob() const;
    float get_npc_drop_potion_prob() const;
    float get_npc_drop_item_prob() const;
    float get_npc_gold_drop_min_factor() const;
    float get_npc_gold_drop_max_factor() const;

    int get_server_game_loop_sleep_ms() const;
};

#endif
