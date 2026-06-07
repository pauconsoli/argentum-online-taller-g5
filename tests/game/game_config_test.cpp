#include <map>
#include <string>

#include "common/position.h"
#include "gtest/gtest.h"
#include "server/game/player_class.h"
#include "server/game/player_race.h"

// con esto puedo simular el config localmente sin tener que levantar el server
// solo para testear
#define private public
#include "server/game/game_config.h"
#undef private

class ConfigEnvironment: public ::testing::Environment {
 public:
    void SetUp() override {
        GameConfig& config = GameConfig::get_instance();
        config.loaded = true;

        // HUMAN
        config.race_hp_mult[PlayerRace::HUMAN] = 1.0f;
        config.race_mana_mult[PlayerRace::HUMAN] = 1.0f;
        config.race_recovery[PlayerRace::HUMAN] = 1.0f;
        config.race_base_strength[PlayerRace::HUMAN] = 10;
        config.race_base_agility[PlayerRace::HUMAN] = 10;
        config.race_base_intelligence[PlayerRace::HUMAN] = 10;
        config.race_base_constitution[PlayerRace::HUMAN] = 10;

        // ELF
        config.race_hp_mult[PlayerRace::ELF] = 0.8f;
        config.race_mana_mult[PlayerRace::ELF] = 1.4f;
        config.race_recovery[PlayerRace::ELF] = 1.2f;
        config.race_base_strength[PlayerRace::ELF] = 7;
        config.race_base_agility[PlayerRace::ELF] = 14;
        config.race_base_intelligence[PlayerRace::ELF] = 14;
        config.race_base_constitution[PlayerRace::ELF] = 7;

        // DWARF
        config.race_hp_mult[PlayerRace::DWARF] = 1.3f;
        config.race_mana_mult[PlayerRace::DWARF] = 0.7f;
        config.race_recovery[PlayerRace::DWARF] = 0.9f;
        config.race_base_strength[PlayerRace::DWARF] = 14;
        config.race_base_agility[PlayerRace::DWARF] = 6;
        config.race_base_intelligence[PlayerRace::DWARF] = 8;
        config.race_base_constitution[PlayerRace::DWARF] = 14;

        // GNOME
        config.race_hp_mult[PlayerRace::GNOME] = 1.1f;
        config.race_mana_mult[PlayerRace::GNOME] = 1.3f;
        config.race_recovery[PlayerRace::GNOME] = 1.0f;
        config.race_base_strength[PlayerRace::GNOME] = 8;
        config.race_base_agility[PlayerRace::GNOME] = 8;
        config.race_base_intelligence[PlayerRace::GNOME] = 13;
        config.race_base_constitution[PlayerRace::GNOME] = 12;

        // MAGE
        config.class_hp_mult[PlayerClass::MAGE] = 0.7f;
        config.class_mana_mult[PlayerClass::MAGE] = 1.5f;
        config.class_meditation[PlayerClass::MAGE] = 1.5f;
        config.class_bonus_strength[PlayerClass::MAGE] = 0;
        config.class_bonus_agility[PlayerClass::MAGE] = 0;
        config.class_bonus_intelligence[PlayerClass::MAGE] = 4;
        config.class_bonus_constitution[PlayerClass::MAGE] = -2;

        // CLERIC
        config.class_hp_mult[PlayerClass::CLERIC] = 1.0f;
        config.class_mana_mult[PlayerClass::CLERIC] = 1.2f;
        config.class_meditation[PlayerClass::CLERIC] = 1.0f;
        config.class_bonus_strength[PlayerClass::CLERIC] = 0;
        config.class_bonus_agility[PlayerClass::CLERIC] = 0;
        config.class_bonus_intelligence[PlayerClass::CLERIC] = 2;
        config.class_bonus_constitution[PlayerClass::CLERIC] = 1;

        // PALADIN
        config.class_hp_mult[PlayerClass::PALADIN] = 1.3f;
        config.class_mana_mult[PlayerClass::PALADIN] = 0.7f;
        config.class_meditation[PlayerClass::PALADIN] = 0.6f;
        config.class_bonus_strength[PlayerClass::PALADIN] = 3;
        config.class_bonus_agility[PlayerClass::PALADIN] = 0;
        config.class_bonus_intelligence[PlayerClass::PALADIN] = -2;
        config.class_bonus_constitution[PlayerClass::PALADIN] = 2;

        // WARRIOR
        config.class_hp_mult[PlayerClass::WARRIOR] = 1.5f;
        config.class_mana_mult[PlayerClass::WARRIOR] = 0.0f;
        config.class_meditation[PlayerClass::WARRIOR] = 0.0f;
        config.class_bonus_strength[PlayerClass::WARRIOR] = 4;
        config.class_bonus_agility[PlayerClass::WARRIOR] = 1;
        config.class_bonus_intelligence[PlayerClass::WARRIOR] = -3;
        config.class_bonus_constitution[PlayerClass::WARRIOR] = 3;

        // World
        config.spawn_position = Position{11, 5};
        config.initial_player_level = 1;

        // Inventory
        config.max_inventory_items = 20;

        // Gold
        config.gold_max_safe_base = 100.0f;
        config.gold_max_safe_exp = 1.1f;
        config.gold_excess_factor = 0.5f;
        config.npc_gold_drop_factor = 0.2f;

        // Experience
        config.level_limit_base = 1000.0f;
        config.level_limit_exp = 1.8f;
        config.kill_bonus_factor = 0.1f;
        config.death_exp_loss_mult = 0.4f;

        // Combat
        config.critical_chance = 0.05f;
        config.evasion_threshold = 0.001f;
        config.newbie_max_level = 12;
        config.max_level_difference = 10;

        // Clan
        config.clan_max_members = 16;
        config.clan_min_level_to_found = 6;

        // NPC
        config.npc_drop_nothing_prob = 0.80f;
        config.npc_drop_gold_prob = 0.08f;
        config.npc_drop_potion_prob = 0.01f;
        config.npc_drop_item_prob = 0.01f;
        config.npc_gold_drop_min_factor = 0.01f;
        config.npc_gold_drop_max_factor = 0.20f;

        // Server
        config.server_game_loop_sleep_ms = 33;
    }
};

testing::Environment* const config_env = testing::AddGlobalTestEnvironment(new ConfigEnvironment);

class GameConfigTest: public ::testing::Test {
 protected:
    GameConfig& config = GameConfig::get_instance();
};

TEST_F(GameConfigTest, HumanHPMultiplier) {
    EXPECT_FLOAT_EQ(config.get_race_hp_multiplier(PlayerRace::HUMAN), 1.0f);
}

TEST_F(GameConfigTest, ElfHPMultiplier) {
    EXPECT_FLOAT_EQ(config.get_race_hp_multiplier(PlayerRace::ELF), 0.8f);
}

TEST_F(GameConfigTest, DwarfHPMultiplier) {
    EXPECT_FLOAT_EQ(config.get_race_hp_multiplier(PlayerRace::DWARF), 1.3f);
}

TEST_F(GameConfigTest, GnomeHPMultiplier) {
    EXPECT_FLOAT_EQ(config.get_race_hp_multiplier(PlayerRace::GNOME), 1.1f);
}

TEST_F(GameConfigTest, HumanManaMultiplier) {
    EXPECT_FLOAT_EQ(config.get_race_mana_multiplier(PlayerRace::HUMAN), 1.0f);
}

TEST_F(GameConfigTest, ElfManaMultiplier) {
    EXPECT_FLOAT_EQ(config.get_race_mana_multiplier(PlayerRace::ELF), 1.4f);
}

TEST_F(GameConfigTest, DwarfManaMultiplier) {
    EXPECT_FLOAT_EQ(config.get_race_mana_multiplier(PlayerRace::DWARF), 0.7f);
}

TEST_F(GameConfigTest, GnomeManaMultiplier) {
    EXPECT_FLOAT_EQ(config.get_race_mana_multiplier(PlayerRace::GNOME), 1.3f);
}

TEST_F(GameConfigTest, MageHPMultiplier) {
    EXPECT_FLOAT_EQ(config.get_class_hp_multiplier(PlayerClass::MAGE), 0.7f);
}

TEST_F(GameConfigTest, ClericHPMultiplier) {
    EXPECT_FLOAT_EQ(config.get_class_hp_multiplier(PlayerClass::CLERIC), 1.0f);
}

TEST_F(GameConfigTest, PaladinHPMultiplier) {
    EXPECT_FLOAT_EQ(config.get_class_hp_multiplier(PlayerClass::PALADIN), 1.3f);
}

TEST_F(GameConfigTest, WarriorHPMultiplier) {
    EXPECT_FLOAT_EQ(config.get_class_hp_multiplier(PlayerClass::WARRIOR), 1.5f);
}

TEST_F(GameConfigTest, MageManaMultiplier) {
    EXPECT_FLOAT_EQ(config.get_class_mana_multiplier(PlayerClass::MAGE), 1.5f);
}

TEST_F(GameConfigTest, ClericManaMultiplier) {
    EXPECT_FLOAT_EQ(config.get_class_mana_multiplier(PlayerClass::CLERIC), 1.2f);
}

TEST_F(GameConfigTest, PaladinManaMultiplier) {
    EXPECT_FLOAT_EQ(config.get_class_mana_multiplier(PlayerClass::PALADIN), 0.7f);
}

TEST_F(GameConfigTest, WarriorManaMultiplier) {
    EXPECT_FLOAT_EQ(config.get_class_mana_multiplier(PlayerClass::WARRIOR), 0.0f);
}

TEST_F(GameConfigTest, WarriorStrongButNoMagic) {
    EXPECT_FLOAT_EQ(config.get_class_mana_multiplier(PlayerClass::WARRIOR), 0.0f);
    EXPECT_GT(config.get_class_hp_multiplier(PlayerClass::WARRIOR),
              config.get_class_hp_multiplier(PlayerClass::MAGE));
}

TEST_F(GameConfigTest, MageWeakButMagicalPower) {
    EXPECT_LT(config.get_class_hp_multiplier(PlayerClass::MAGE),
              config.get_class_hp_multiplier(PlayerClass::WARRIOR));
    EXPECT_GT(config.get_class_mana_multiplier(PlayerClass::MAGE),
              config.get_class_mana_multiplier(PlayerClass::WARRIOR));
}

TEST_F(GameConfigTest, DwarfStrongButLittleMana) {
    EXPECT_GT(config.get_race_hp_multiplier(PlayerRace::DWARF),
              config.get_race_hp_multiplier(PlayerRace::HUMAN));
    EXPECT_LT(config.get_race_mana_multiplier(PlayerRace::DWARF),
              config.get_race_mana_multiplier(PlayerRace::HUMAN));
}

TEST_F(GameConfigTest, ElfMagicalButFragile) {
    EXPECT_LT(config.get_race_hp_multiplier(PlayerRace::ELF),
              config.get_race_hp_multiplier(PlayerRace::HUMAN));
    EXPECT_GT(config.get_race_mana_multiplier(PlayerRace::ELF),
              config.get_race_mana_multiplier(PlayerRace::HUMAN));
}

TEST_F(GameConfigTest, RecoveryAndLimits) {
    EXPECT_FLOAT_EQ(config.get_race_recovery_factor(PlayerRace::HUMAN), 1.0f);
    EXPECT_EQ(config.get_max_inventory_items(), 20);
    EXPECT_FLOAT_EQ(config.get_gold_max_safe_base(), 100.0f);
    EXPECT_FLOAT_EQ(config.get_gold_max_safe_exp(), 1.1f);
    EXPECT_FLOAT_EQ(config.get_gold_excess_factor(), 0.5f);
    EXPECT_FLOAT_EQ(config.get_npc_gold_drop_factor(), 0.2f);
    EXPECT_FLOAT_EQ(config.get_level_limit_base(), 1000.0f);
    EXPECT_FLOAT_EQ(config.get_level_limit_exp(), 1.8f);
    EXPECT_FLOAT_EQ(config.get_kill_bonus_factor(), 0.1f);
    EXPECT_FLOAT_EQ(config.get_death_exp_loss_mult(), 0.4f);
}

TEST_F(GameConfigTest, ClassMeditationFactor) {
    EXPECT_FLOAT_EQ(config.get_class_meditation_factor(PlayerClass::MAGE), 1.5f);
    EXPECT_FLOAT_EQ(config.get_class_meditation_factor(PlayerClass::WARRIOR), 0.0f);
}
