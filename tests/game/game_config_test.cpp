#include "gtest/gtest.h"
#include "server/game/game_config.h"
#include "server/game/player_race.h"
#include "server/game/player_class.h"

class GameConfigTest : public ::testing::Test {
protected:
    GameConfig& config = GameConfig::get_instance();
};

TEST_F(GameConfigTest, SingletonInstance) {
    GameConfig& config1 = GameConfig::get_instance();
    GameConfig& config2 = GameConfig::get_instance();
    
    EXPECT_EQ(&config1, &config2);
}

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
