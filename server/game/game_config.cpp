#include "game_config.h"

// Ahora los valores estan hardcodeados
// TODO: que se carguen del TOML

GameConfig& GameConfig::get_instance() {
    static GameConfig instance;
    return instance;
}

float GameConfig::get_race_hp_multiplier(PlayerRace race) const {
    switch (race) {
        case PlayerRace::HUMAN:
            return 1.0f;

        case PlayerRace::ELF:
            return 0.8f;

        case PlayerRace::DWARF:
            return 1.3f;

        case PlayerRace::GNOME:
            return 1.1f;
    }

    return 1.0f;
}

float GameConfig::get_race_mana_multiplier(PlayerRace race) const {
    switch (race) {
        case PlayerRace::HUMAN:
            return 1.0f;

        case PlayerRace::ELF:
            return 1.4f;

        case PlayerRace::DWARF:
            return 0.7f;

        case PlayerRace::GNOME:
            return 1.3f;
    }

    return 1.0f;
}

float GameConfig::get_class_hp_multiplier(
        PlayerClass player_class) const {

    switch (player_class) {
        case PlayerClass::MAGE:
            return 0.7f;

        case PlayerClass::CLERIC:
            return 1.0f;

        case PlayerClass::PALADIN:
            return 1.3f;

        case PlayerClass::WARRIOR:
            return 1.5f;
    }

    return 1.0f;
}

float GameConfig::get_class_mana_multiplier(
        PlayerClass player_class) const {

    switch (player_class) {
        case PlayerClass::MAGE:
            return 1.5f;

        case PlayerClass::CLERIC:
            return 1.2f;

        case PlayerClass::PALADIN:
            return 0.7f;

        case PlayerClass::WARRIOR:
            return 0.0f;
    }

    return 1.0f;
}