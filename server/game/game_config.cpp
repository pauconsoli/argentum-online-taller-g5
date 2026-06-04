#include "game_config.h"

#include <stdexcept>
#include <string>

// solo para que cppchek no me tire el warning, los valores verdaderos se cargan desde el toml en el
// loader
GameConfig::GameConfig():
    loaded(false),
    spawn_position{},
    max_inventory_items(0),
    gold_max_safe_base(0.0f),
    gold_max_safe_exp(0.0f),
    gold_excess_factor(0.0f),
    npc_gold_drop_factor(0.0f),
    level_limit_base(0.0f),
    level_limit_exp(0.0f),
    kill_bonus_factor(0.0f),
    critical_chance(0.0f),
    dodge_threshold(0.0f),
    newbie_max_level(0),
    max_level_difference(0),
    clan_max_members(0),
    clan_min_level_to_found(0),
    npc_drop_nothing_prob(0.0f),
    npc_drop_gold_prob(0.0f),
    npc_drop_potion_prob(0.0f),
    npc_drop_item_prob(0.0f),
    npc_gold_drop_min_factor(0.0f),
    npc_gold_drop_max_factor(0.0f),
    server_game_loop_sleep_ms(0) {}

static float lookup(const std::map<PlayerRace, float>& m, PlayerRace key, const char* field) {
    auto it = m.find(key);
    if (it == m.end())
        throw std::runtime_error(std::string("GameConfig: raza sin '") + field + "' cargado");
    return it->second;
}

static float lookup(const std::map<PlayerClass, float>& m, PlayerClass key, const char* field) {
    auto it = m.find(key);
    if (it == m.end())
        throw std::runtime_error(std::string("GameConfig: clase sin '") + field + "' cargado");
    return it->second;
}

static int lookup(const std::map<PlayerRace, int>& m, PlayerRace key, const char* field) {
    auto it = m.find(key);
    if (it == m.end())
        throw std::runtime_error(std::string("GameConfig: raza sin '") + field + "' cargado");
    return it->second;
}

static int lookup(const std::map<PlayerClass, int>& m, PlayerClass key, const char* field) {
    auto it = m.find(key);
    if (it == m.end())
        throw std::runtime_error(std::string("GameConfig: clase sin '") + field + "' cargado");
    return it->second;
}


GameConfig& GameConfig::get_instance() {
    static GameConfig instance;
    return instance;
}

static void check_loaded(
    bool loaded) {  // para verificar que se haya cargado el config antes de usarlo
    if (!loaded)
        throw std::runtime_error("GameConfig: get_instance() usado antes de llamar a "
                                 "GameConfigLoader::load()");
}

float GameConfig::get_race_hp_multiplier(PlayerRace race) const {
    check_loaded(loaded);
    return lookup(race_hp_mult, race, "hp_multiplier");
}

float GameConfig::get_race_mana_multiplier(PlayerRace race) const {
    check_loaded(loaded);
    return lookup(race_mana_mult, race, "mana_multiplier");
}

float GameConfig::get_race_recovery_factor(PlayerRace race) const {
    check_loaded(loaded);
    return lookup(race_recovery, race, "recovery_factor");
}

int GameConfig::get_race_strength(PlayerRace race) const {
    check_loaded(loaded);
    return lookup(race_base_strength, race, "base_strength");
}

int GameConfig::get_race_agility(PlayerRace race) const {
    check_loaded(loaded);
    return lookup(race_base_agility, race, "base_agility");
}

int GameConfig::get_race_intelligence(PlayerRace race) const {
    check_loaded(loaded);
    return lookup(race_base_intelligence, race, "base_intelligence");
}

int GameConfig::get_race_constitution(PlayerRace race) const {
    check_loaded(loaded);
    return lookup(race_base_constitution, race, "base_constitution");
}

int GameConfig::get_class_bonus_strength(PlayerClass cls) const {
    check_loaded(loaded);
    return lookup(class_bonus_strength, cls, "bonus_strength");
}

int GameConfig::get_class_bonus_agility(PlayerClass cls) const {
    check_loaded(loaded);
    return lookup(class_bonus_agility, cls, "bonus_agility");
}

int GameConfig::get_class_bonus_intelligence(PlayerClass cls) const {
    check_loaded(loaded);
    return lookup(class_bonus_intelligence, cls, "bonus_intelligence");
}

int GameConfig::get_class_bonus_constitution(PlayerClass cls) const {
    check_loaded(loaded);
    return lookup(class_bonus_constitution, cls, "bonus_constitution");
}

float GameConfig::get_class_hp_multiplier(PlayerClass cls) const {
    check_loaded(loaded);
    return lookup(class_hp_mult, cls, "hp_multiplier");
}

float GameConfig::get_class_mana_multiplier(PlayerClass cls) const {
    check_loaded(loaded);
    return lookup(class_mana_mult, cls, "mana_multiplier");
}

float GameConfig::get_class_meditation_factor(PlayerClass cls) const {
    check_loaded(loaded);
    return lookup(class_meditation, cls, "meditation_factor");
}

Position GameConfig::get_spawn_position() const {
    check_loaded(loaded);
    return spawn_position;
}

int GameConfig::get_max_inventory_items() const {
    check_loaded(loaded);
    return max_inventory_items;
}

float GameConfig::get_gold_max_safe_base() const {
    check_loaded(loaded);
    return gold_max_safe_base;
}
float GameConfig::get_gold_max_safe_exp() const {
    check_loaded(loaded);
    return gold_max_safe_exp;
}
float GameConfig::get_gold_excess_factor() const {
    check_loaded(loaded);
    return gold_excess_factor;
}
float GameConfig::get_npc_gold_drop_factor() const {
    check_loaded(loaded);
    return npc_gold_drop_factor;
}

float GameConfig::get_level_limit_base() const {
    check_loaded(loaded);
    return level_limit_base;
}
float GameConfig::get_level_limit_exp() const {
    check_loaded(loaded);
    return level_limit_exp;
}
float GameConfig::get_kill_bonus_factor() const {
    check_loaded(loaded);
    return kill_bonus_factor;
}

float GameConfig::get_critical_chance() const {
    check_loaded(loaded);
    return critical_chance;
}
float GameConfig::get_dodge_threshold() const {
    check_loaded(loaded);
    return dodge_threshold;
}
int GameConfig::get_newbie_max_level() const {
    check_loaded(loaded);
    return newbie_max_level;
}
int GameConfig::get_max_level_difference() const {
    check_loaded(loaded);
    return max_level_difference;
}

int GameConfig::get_clan_max_members() const {
    check_loaded(loaded);
    return clan_max_members;
}
int GameConfig::get_clan_min_level_to_found() const {
    check_loaded(loaded);
    return clan_min_level_to_found;
}

float GameConfig::get_npc_drop_nothing_prob() const {
    check_loaded(loaded);
    return npc_drop_nothing_prob;
}
float GameConfig::get_npc_drop_gold_prob() const {
    check_loaded(loaded);
    return npc_drop_gold_prob;
}
float GameConfig::get_npc_drop_potion_prob() const {
    check_loaded(loaded);
    return npc_drop_potion_prob;
}
float GameConfig::get_npc_drop_item_prob() const {
    check_loaded(loaded);
    return npc_drop_item_prob;
}
float GameConfig::get_npc_gold_drop_min_factor() const {
    check_loaded(loaded);
    return npc_gold_drop_min_factor;
}
float GameConfig::get_npc_gold_drop_max_factor() const {
    check_loaded(loaded);
    return npc_gold_drop_max_factor;
}

int GameConfig::get_server_game_loop_sleep_ms() const {
    check_loaded(loaded);
    return server_game_loop_sleep_ms;
}
