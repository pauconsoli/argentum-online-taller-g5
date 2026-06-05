#include "game_formulas.h"

#include <cmath>
#include <memory>
#include <string>

#include "game_config.h"
#include "player.h"

// casteo a int porque tiene más sentido que los resultados sean enteros, pero se puede cambiar

int GameFormulas::calculate_max_hp(const Player& player) {
    const GameConfig& config = GameConfig::get_instance();

    float class_multiplier = config.get_class_hp_multiplier(player.get_class());
    float race_multiplier = config.get_race_hp_multiplier(player.get_race());

    return static_cast<int>(player.get_constitution() * class_multiplier * race_multiplier *
                            player.get_level());
}

int GameFormulas::calculate_max_mana(const Player& player) {
    if (!player.can_cast_magic()) {
        return 0;
    }

    const GameConfig& config = GameConfig::get_instance();

    float class_multiplier = config.get_class_mana_multiplier(player.get_class());
    float race_multiplier = config.get_race_mana_multiplier(player.get_race());

    return static_cast<int>(player.get_intelligence() * class_multiplier * race_multiplier *
                            player.get_level());
}

int GameFormulas::calculate_max_gold(const Player& player) {
    const GameConfig& config = GameConfig::get_instance();
    return static_cast<int>(config.get_gold_max_safe_base() *
                            std::pow(player.get_level(), config.get_gold_max_safe_exp()));
}

int GameFormulas::calculate_health_recovery(const Player& player, float seconds) {
    const GameConfig& config = GameConfig::get_instance();
    float recovery_factor = config.get_race_recovery_factor(player.get_race());
    return static_cast<int>(recovery_factor * seconds);
}

int GameFormulas::calculate_time_mana_recovery(const Player& player, float seconds) {
    if (!player.can_cast_magic()) {
        return 0;
    }
    const GameConfig& config = GameConfig::get_instance();
    float recovery_factor = config.get_race_recovery_factor(player.get_race());
    return static_cast<int>(recovery_factor * seconds);
}

int GameFormulas::calculate_meditation_mana_recovery(const Player& player, float seconds) {
    if (!player.can_cast_magic()) {
        return 0;
    }
    const GameConfig& config = GameConfig::get_instance();
    float meditation_factor = config.get_class_meditation_factor(player.get_class());
    return static_cast<int>(meditation_factor * player.get_intelligence() * seconds);
}

BaseStats GameFormulas::calculate_base_stats(PlayerRace race, PlayerClass klass) {
    const GameConfig& config = GameConfig::get_instance();
    return {config.get_race_strength(race) + config.get_class_bonus_strength(klass),
            config.get_race_agility(race) + config.get_class_bonus_agility(klass),
            config.get_race_intelligence(race) + config.get_class_bonus_intelligence(klass),
            config.get_race_constitution(race) + config.get_class_bonus_constitution(klass)};
}

std::unique_ptr<Player> GameFormulas::create_initial_player(uint32_t id, const std::string& name,
                                                            PlayerRace race, PlayerClass klass,
                                                            const Position& pos) {
    BaseStats stats = calculate_base_stats(race, klass);

    const GameConfig& config = GameConfig::get_instance();
    int new_player_level = config.get_initial_player_level();

    auto player =
        std::make_unique<Player>(id, name, race, klass, new_player_level, 0, 0, stats.strength,
                                 stats.agility, stats.intelligence, stats.constitution, pos);

    int max_hp = calculate_max_hp(*player);
    int max_mana = calculate_max_mana(*player);

    player->set_initial_stats(
        max_hp <= 0 ? 1 : max_hp,
        max_mana);  // asegurar que el jugador tenga al menos 1 de hp para no morir al crearse

    return player;
}
