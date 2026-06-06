#include "server/game/player.h"

#include "common/position.h"
#include "gtest/gtest.h"

class PlayerTest: public ::testing::Test {
 protected:
    Position position{5, 10};

    Player mage{1,  "Merlin", PlayerRace::HUMAN, PlayerClass::MAGE, 5, 100, 80, 10, 8,
                16, 15,       position};
    Player warrior{2, "Conan", PlayerRace::DWARF, PlayerClass::WARRIOR, 5, 150, 0, 18, 7,
                   8, 18,      position};
    Player cleric{3,  "Priest", PlayerRace::HUMAN, PlayerClass::CLERIC, 5, 100, 60, 12, 9,
                  14, 14,       position};
    Player paladin{4,  "Knight", PlayerRace::HUMAN, PlayerClass::PALADIN, 5, 130, 40, 15, 10,
                   12, 16,       position};
};

TEST_F(PlayerTest, ConstructorInitializesPlayer) {
    EXPECT_EQ(mage.get_id(), 1u);
    EXPECT_EQ(mage.get_name(), "Merlin");
    EXPECT_EQ(mage.get_race(), PlayerRace::HUMAN);
    EXPECT_EQ(mage.get_class(), PlayerClass::MAGE);
    EXPECT_EQ(mage.get_level(), 5);
    EXPECT_EQ(mage.get_gold(), 0u);
    EXPECT_EQ(mage.get_experience(), 0u);
    EXPECT_FALSE(mage.is_meditating());
}

TEST_F(PlayerTest, MageCanCastMagic) {
    EXPECT_TRUE(mage.can_cast_magic());
}

TEST_F(PlayerTest, WarriorCannotCastMagic) {
    EXPECT_FALSE(warrior.can_cast_magic());
}

TEST_F(PlayerTest, ClericCanCastMagic) {
    EXPECT_TRUE(cleric.can_cast_magic());
}

TEST_F(PlayerTest, PaladinCanCastMagic) {
    EXPECT_TRUE(paladin.can_cast_magic());
}

TEST_F(PlayerTest, MoveChangesPosition) {
    Position new_pos{15, 20};
    mage.move(new_pos);

    const Position& pos = mage.get_position();
    EXPECT_EQ(pos.x, 15);
    EXPECT_EQ(pos.y, 20);
}

TEST_F(PlayerTest, AddGold) {
    mage.add_gold(100);
    EXPECT_EQ(mage.get_gold(), 100u);

    mage.add_gold(50);
    EXPECT_EQ(mage.get_gold(), 150u);
}

TEST_F(PlayerTest, RemoveGoldSuccess) {
    mage.add_gold(200);
    EXPECT_TRUE(mage.remove_gold(100));
    EXPECT_EQ(mage.get_gold(), 100u);
}

TEST_F(PlayerTest, RemoveGoldFailsIfInsufficient) {
    mage.add_gold(50);
    EXPECT_FALSE(mage.remove_gold(100));
    EXPECT_EQ(mage.get_gold(), 50u);
}

TEST_F(PlayerTest, RemoveGoldExact) {
    mage.add_gold(100);
    EXPECT_TRUE(mage.remove_gold(100));
    EXPECT_EQ(mage.get_gold(), 0u);
}

TEST_F(PlayerTest, RemoveGoldFromZero) {
    EXPECT_FALSE(mage.remove_gold(10));
    EXPECT_EQ(mage.get_gold(), 0u);
}

TEST_F(PlayerTest, AddExperience) {
    mage.add_experience(1000);
    EXPECT_EQ(mage.get_experience(), 1000u);

    mage.add_experience(500);
    EXPECT_EQ(mage.get_experience(), 1500u);
}

TEST_F(PlayerTest, StartMeditationMage) {
    EXPECT_FALSE(mage.is_meditating());
    mage.start_meditating();
    EXPECT_TRUE(mage.is_meditating());
}

TEST_F(PlayerTest, StopMeditationMage) {
    mage.start_meditating();
    EXPECT_TRUE(mage.is_meditating());
    mage.stop_meditating();
    EXPECT_FALSE(mage.is_meditating());
}

TEST_F(PlayerTest, WarriorCannotMeditate) {
    EXPECT_FALSE(warrior.is_meditating());
    warrior.start_meditating();
    EXPECT_FALSE(warrior.is_meditating());
}

TEST_F(PlayerTest, ClericCanMeditate) {
    EXPECT_FALSE(cleric.is_meditating());
    cleric.start_meditating();
    EXPECT_TRUE(cleric.is_meditating());
}

TEST_F(PlayerTest, InheritsCharacterMethods) {
    EXPECT_EQ(mage.get_current_hp(), 100);
    mage.receive_damage(30);
    EXPECT_EQ(mage.get_current_hp(), 70);

    mage.heal(10);
    EXPECT_EQ(mage.get_current_hp(), 80);
}

TEST_F(PlayerTest, CanDieAndBecomeGhost) {
    mage.receive_damage(100);
    EXPECT_TRUE(mage.is_dead());
}

TEST_F(PlayerTest, SetInitialStats) {
    mage.set_initial_stats(200, 150);
    EXPECT_EQ(mage.get_max_hp(), 200);
    EXPECT_EQ(mage.get_current_hp(), 200);
    EXPECT_EQ(mage.get_max_mana(), 150);
    EXPECT_EQ(mage.get_current_mana(), 150);
}

TEST_F(PlayerTest, RestoreAndConsumeMana) {
    mage.set_initial_stats(100, 100);
    mage.consume_mana(30);
    EXPECT_EQ(mage.get_current_mana(), 70);

    mage.consume_mana(100);
    EXPECT_EQ(mage.get_current_mana(), 0);

    mage.restore_mana(20);
    EXPECT_EQ(mage.get_current_mana(), 20);

    mage.restore_mana(100);
    EXPECT_EQ(mage.get_current_mana(), 100);
}

TEST_F(PlayerTest, WarriorCannotRestoreOrConsumeMana) {
    warrior.set_initial_stats(100, 0);
    warrior.restore_mana(10);
    EXPECT_EQ(warrior.get_current_mana(), 0);
    warrior.consume_mana(10);
    EXPECT_EQ(warrior.get_current_mana(), 0);
}

TEST_F(PlayerTest, ValidateAttackFrom) {
    // mage nivel 5 (newbie)
    EXPECT_FALSE(mage.validate_attack_from(15));

    mage.set_level(20);
    EXPECT_TRUE(mage.validate_attack_from(25));   // Válido
    EXPECT_FALSE(mage.validate_attack_from(35));  // Diferencia de nivel > 10
}
