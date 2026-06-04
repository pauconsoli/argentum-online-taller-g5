#include "game_config_loader.h"

#include <stdexcept>
#include <string>

#include <toml++/toml.hpp>

#include "server/game/game_config.h"
#include "server/game/player_class.h"
#include "server/game/player_race.h"

static float require_float(const toml::table& t, std::string_view section, std::string_view key) {
    auto val = t[key].value<float>();
    if (!val)
        throw std::runtime_error("game_config.toml: falta '" + std::string(key) + "' en [" +
                                 std::string(section) + "]");
    return *val;
}

static int require_int(const toml::table& t, std::string_view section, std::string_view key) {
    auto val = t[key].value<int>();
    if (!val)
        throw std::runtime_error("game_config.toml: falta '" + std::string(key) + "' en [" +
                                 std::string(section) + "]");
    return *val;
}


void GameConfigLoader::load(const std::string& config_path) {
    auto root = toml::parse_file(config_path);
    GameConfig& gc = GameConfig::get_instance();

    // LAMBDAS para no repetir código y así tienen acceso a atributos privados de GameConfig
    auto load_race = [&](PlayerRace race, std::string_view key) {
        const auto* subtable = root["race"][key].as_table();
        if (!subtable)
            throw std::runtime_error("game_config.toml: falta la sección [race." +
                                     std::string(key) + "]");
        std::string section = "race." + std::string(key);
        gc.race_hp_mult[race] = require_float(*subtable, section, "hp_multiplier");
        gc.race_mana_mult[race] = require_float(*subtable, section, "mana_multiplier");
        gc.race_recovery[race] = require_float(*subtable, section, "recovery_factor");
        gc.race_base_strength[race] = require_int(*subtable, section, "strength");
        gc.race_base_agility[race] = require_int(*subtable, section, "agility");
        gc.race_base_intelligence[race] = require_int(*subtable, section, "intelligence");
        gc.race_base_constitution[race] = require_int(*subtable, section, "constitution");
    };

    auto load_class = [&](PlayerClass cls, std::string_view key) {
        const auto* subtable = root["class"][key].as_table();
        if (!subtable)
            throw std::runtime_error("game_config.toml: falta la sección [class." +
                                     std::string(key) + "]");
        std::string section = "class." + std::string(key);
        gc.class_hp_mult[cls] = require_float(*subtable, section, "hp_multiplier");
        gc.class_mana_mult[cls] = require_float(*subtable, section, "mana_multiplier");
        gc.class_meditation[cls] = require_float(*subtable, section, "meditation_factor");
        gc.class_bonus_strength[cls] = require_int(*subtable, section, "strength_bonus");
        gc.class_bonus_agility[cls] = require_int(*subtable, section, "agility_bonus");
        gc.class_bonus_intelligence[cls] = require_int(*subtable, section, "intelligence_bonus");
        gc.class_bonus_constitution[cls] = require_int(*subtable, section, "constitution_bonus");
    };

    load_race(PlayerRace::HUMAN, "human");
    load_race(PlayerRace::ELF, "elf");
    load_race(PlayerRace::DWARF, "dwarf");
    load_race(PlayerRace::GNOME, "gnome");

    load_class(PlayerClass::MAGE, "mage");
    load_class(PlayerClass::CLERIC, "cleric");
    load_class(PlayerClass::PALADIN, "paladin");
    load_class(PlayerClass::WARRIOR, "warrior");

    const auto* inv = root["inventory"].as_table();
    if (!inv)
        throw std::runtime_error("game_config.toml: falta la sección [inventory]");
    gc.max_inventory_items = require_int(*inv, "inventory", "max_items");


    const auto* world = root["world"].as_table();
    if (!world)
        throw std::runtime_error("game_config.toml: falta la sección [world]");
    gc.spawn_position.x = require_int(*world, "world", "spawn_x");
    gc.spawn_position.y = require_int(*world, "world", "spawn_y");


    const auto* gold = root["gold"].as_table();
    if (!gold)
        throw std::runtime_error("game_config.toml: falta la sección [gold]");
    gc.gold_max_safe_base = require_float(*gold, "gold", "max_safe_base");
    gc.gold_max_safe_exp = require_float(*gold, "gold", "max_safe_exp");
    gc.gold_excess_factor = require_float(*gold, "gold", "excess_factor");
    gc.npc_gold_drop_factor = require_float(*gold, "gold", "npc_drop_factor");

    const auto* exp = root["experience"].as_table();
    if (!exp)
        throw std::runtime_error("game_config.toml: falta la sección [experience]");
    gc.level_limit_base = require_float(*exp, "experience", "level_limit_base");
    gc.level_limit_exp = require_float(*exp, "experience", "level_limit_exp");
    gc.kill_bonus_factor = require_float(*exp, "experience", "kill_bonus_factor");

    const auto* combat = root["combat"].as_table();
    if (!combat)
        throw std::runtime_error("game_config.toml: falta la sección [combat]");
    gc.critical_chance = require_float(*combat, "combat", "critical_chance");
    gc.dodge_threshold = require_float(*combat, "combat", "dodge_threshold");
    gc.newbie_max_level = require_int(*combat, "combat", "newbie_max_level");
    gc.max_level_difference = require_int(*combat, "combat", "max_level_difference");

    const auto* clan = root["clan"].as_table();
    if (!clan)
        throw std::runtime_error("game_config.toml: falta la sección [clan]");
    gc.clan_max_members = require_int(*clan, "clan", "max_members");
    gc.clan_min_level_to_found = require_int(*clan, "clan", "min_level_to_found");

    const auto* npc = root["npc"].as_table();
    if (!npc)
        throw std::runtime_error("game_config.toml: falta la sección [npc]");
    gc.npc_drop_nothing_prob = require_float(*npc, "npc", "drop_nothing_prob");
    gc.npc_drop_gold_prob = require_float(*npc, "npc", "drop_gold_prob");
    gc.npc_drop_potion_prob = require_float(*npc, "npc", "drop_potion_prob");
    gc.npc_drop_item_prob = require_float(*npc, "npc", "drop_item_prob");
    gc.npc_gold_drop_min_factor = require_float(*npc, "npc", "gold_drop_min_factor");
    gc.npc_gold_drop_max_factor = require_float(*npc, "npc", "gold_drop_max_factor");

    const auto* server = root["server"].as_table();
    if (!server)
        throw std::runtime_error("game_config.toml: falta la sección [server]");
    gc.server_game_loop_sleep_ms = require_int(*server, "server", "game_loop_sleep_ms");

    gc.loaded = true;
}
