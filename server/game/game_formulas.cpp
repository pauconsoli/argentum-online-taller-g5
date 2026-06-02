#include "game_formulas.h"

#include <cmath>

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
