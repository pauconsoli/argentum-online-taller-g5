#include "game_formulas.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <random>
#include <string>

#include "game_config.h"
#include "player.h"
#include "server/game/character.h"
#include "server/game/items/defensive_item.h"
#include "server/game/items/weapon.h"

// casteo a int porque tiene más sentido que los resultados sean enteros, pero se puede cambiar

// UTILS

int GameFormulas::get_random_int(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}

float GameFormulas::get_random_float(float min, float max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distrib(min, max);
    return distrib(gen);
}

// STATS

// VidaMax = Constitución * FClaseVida * FRazaVida * Nivel
int GameFormulas::calculate_max_hp(const Player& player) {
    const GameConfig& config = GameConfig::get_instance();

    float class_multiplier = config.get_class_hp_multiplier(player.get_class());
    float race_multiplier = config.get_race_hp_multiplier(player.get_race());

    return static_cast<int>(player.get_constitution() * class_multiplier * race_multiplier *
                            player.get_level());
}

// ManaMax = Inteligencia * FClaseMana * FRazaMana * Nivel
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

// OroMax = 100 * Nivel^1.1
int GameFormulas::calculate_max_gold(const Player& player) {
    const GameConfig& config = GameConfig::get_instance();
    return static_cast<int>(config.get_gold_max_safe_base() *
                            std::pow(player.get_level(), config.get_gold_max_safe_exp()));
}

// Limite = 1000 * Nivel^1.8
uint64_t GameFormulas::calculate_level_up_limit(int level) {
    const GameConfig& config = GameConfig::get_instance();
    return static_cast<uint64_t>(config.get_level_limit_base() *
                                 std::pow(level, config.get_level_limit_exp()));
}

BaseStats GameFormulas::calculate_base_stats(PlayerRace race, PlayerClass klass) {
    const GameConfig& config = GameConfig::get_instance();
    return {config.get_race_strength(race) + config.get_class_bonus_strength(klass),
            config.get_race_agility(race) + config.get_class_bonus_agility(klass),
            config.get_race_intelligence(race) + config.get_class_bonus_intelligence(klass),
            config.get_race_constitution(race) + config.get_class_bonus_constitution(klass)};
}


// RECOVERY

// Vida = FRazaRecuperacion * segundos
int GameFormulas::calculate_health_recovery(const Player& player, float seconds) {
    const GameConfig& config = GameConfig::get_instance();
    float recovery_factor = config.get_race_recovery_factor(player.get_race());
    return static_cast<int>(recovery_factor * seconds);
}

// Mana = FRazaRecuperacion * segundos
int GameFormulas::calculate_time_mana_recovery(const Player& player, float seconds) {
    if (!player.can_cast_magic()) {
        return 0;
    }
    const GameConfig& config = GameConfig::get_instance();
    float recovery_factor = config.get_race_recovery_factor(player.get_race());
    return static_cast<int>(recovery_factor * seconds);
}

// Mana = FClaseMeditacion * Inteligencia * segundos
int GameFormulas::calculate_meditation_mana_recovery(const Player& player, float seconds) {
    if (!player.can_cast_magic()) {
        return 0;
    }
    const GameConfig& config = GameConfig::get_instance();
    float meditation_factor = config.get_class_meditation_factor(player.get_class());
    return static_cast<int>(meditation_factor * player.get_intelligence() * seconds);
}


// SPAWN

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


// ATAQUE

bool GameFormulas::can_attack_by_level(int attacker_level, int target_level) {
    const GameConfig& config = GameConfig::get_instance();
    int newbie_max_level = config.get_newbie_max_level();
    int max_level_difference = config.get_max_level_difference();

    if (attacker_level <= newbie_max_level || target_level <= newbie_max_level) {
        return false;
    }

    return std::abs(attacker_level - target_level) <= max_level_difference;
}

// esta fórmula es inventada
int GameFormulas::get_hand_combat_damage(const Player& attacker) {
    return static_cast<int>(
        attacker.get_strength());  // daño base por combate sin arma, se puede ajustar
}

// Crítico si rand(0, 1) < ProbCritico
bool GameFormulas::calculate_critical_attack() {
    const GameConfig& config = GameConfig::get_instance();
    return get_random_float(0.0, 1.0) < config.get_critical_chance();
}

// Daño = Fuerza * rand(DañoArmaMin, DañoArmaMax)
int GameFormulas::calculate_damage(const Player& attacker) {
    Item* item = attacker.get_inventory().get_equipped_item(EquipmentSlot::WEAPON);

    // consultar sobre este cast
    Weapon* weapon = static_cast<Weapon*>(
        item);  // ya sé que el item equipado ahí va a ser un arma si o si, lo garantiza mi diseño

    int min_dmg = weapon ? weapon->get_min_damage() : get_hand_combat_damage(attacker);
    int max_dmg = weapon ? weapon->get_max_damage() : get_hand_combat_damage(attacker);

    int weapon_damage =
        get_random_int(min_dmg, max_dmg);  // ver si usar el get_random_int o get_random_float

    return attacker.get_strength() * weapon_damage;
}

// Esquivar si rand(0, 1) ^ Agilidad < 0.001
bool GameFormulas::calculate_evasion(const Character& target) {
    const GameConfig& config = GameConfig::get_instance();
    float evasion_threshold = config.get_evasion_threshold();
    float evade = std::pow(get_random_float(0.0, 1.0), target.get_agility());
    return evade < evasion_threshold;
}

// Defensa = rand(ArmaduraMin, ArmaduraMax) + rand(EscudoMin, EscudoMax) + rand(CascoMin, CascoMax)
int GameFormulas::calculate_defense(const Player& target) {
    int total_defense = 0;
    const Inventory& inventory = target.get_inventory();

    auto add_defense_from_slot = [&](EquipmentSlot slot) {
        Item* item = inventory.get_equipped_item(slot);
        if (item) {  // consultar tamb
            DefensiveItem* defensive_item = static_cast<DefensiveItem*>(
                item);  // ya sé que el item equipado ahí va a ser un item defensivo si o si, lo
                        // garantiza mi diseño
            total_defense += get_random_int(
                defensive_item->get_min_defense(),
                defensive_item
                    ->get_max_defense());  // ver si usar el get_random_int o get_random_float
        }
    };

    add_defense_from_slot(EquipmentSlot::ARMOR);
    add_defense_from_slot(EquipmentSlot::SHIELD);
    add_defense_from_slot(EquipmentSlot::HELMET);

    return total_defense;
}

// Exp = Daño * max(NivelDelOtro - Nivel + 10, 0)
int GameFormulas::calculate_attack_experience_gain(const Player& attacker,
                                                   const Character& target) {
    int damage = calculate_damage(attacker);
    int level_multiplier = std::max(target.get_level() - attacker.get_level() + 10, 0);
    return static_cast<int>(damage * level_multiplier);
}

// Exp = rand(0, 0.1) * VidaMaxDelOtro * max(NivelDelOtro - Nivel + 10, 0)
int GameFormulas::calculate_kill_experience_gain(const Player& attacker, const Character& target) {
    const GameConfig& config = GameConfig::get_instance();
    int target_max_hp = target.get_max_hp();
    int level_multiplier = std::max(target.get_level() - attacker.get_level() + 10, 0);

    float random_factor = get_random_float(0.0, config.get_kill_bonus_factor());
    return static_cast<int>(random_factor * target_max_hp * level_multiplier);
}

// MUERTE Y DROPS

uint64_t GameFormulas::calculate_player_dropped_gold(const Player& player) {
    uint64_t max_safe_gold = calculate_max_gold(player);
    uint64_t current_gold = player.get_gold();
    return (current_gold > max_safe_gold) ? (current_gold - max_safe_gold) : 0;
}

// Esta fórmula es inventada porque en el enunciado no dice cuanta exp se pierde
// Le asigno un valor del 40% de la exp actual pero se puede ajustar
uint64_t GameFormulas::calculate_player_dropped_experience(const Player& player) {
    return static_cast<uint64_t>(player.get_experience() * 0.40f);
}
