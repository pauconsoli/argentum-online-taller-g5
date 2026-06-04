#include "server/game/character.h"

#include "common/position.h"
#include "gtest/gtest.h"
#include "server/game/player.h"

class TestCharacter: public Character {
 public:
    TestCharacter(uint32_t id, int level, int max_hp, int max_mana, int strength, int agility,
                  int intelligence, int constitution, const Position& position):
        Character(id, level, max_hp, max_mana, strength, agility, intelligence, constitution,
                  position) {}

    bool can_cast_magic() const override {
        return true;
    }
};

class CharacterTest: public ::testing::Test {
 protected:
    Position position{10, 20};
    TestCharacter character{1, 5, 100, 50, 10, 8, 12, 15, position};
};

TEST_F(CharacterTest, ConstructorInitializesCorrectly) {
    EXPECT_EQ(character.get_id(), 1u);
    EXPECT_EQ(character.get_level(), 5);
    EXPECT_EQ(character.get_current_hp(), 100);
    EXPECT_EQ(character.get_max_hp(), 100);
    EXPECT_EQ(character.get_current_mana(), 50);
    EXPECT_EQ(character.get_max_mana(), 50);
    EXPECT_EQ(character.get_strength(), 10);
    EXPECT_EQ(character.get_agility(), 8);
    EXPECT_EQ(character.get_intelligence(), 12);
    EXPECT_EQ(character.get_constitution(), 15);
    EXPECT_FALSE(character.is_dead());
}

TEST_F(CharacterTest, ReceiveDamageReducesHP) {
    character.receive_damage(25);
    EXPECT_EQ(character.get_current_hp(), 75);
    EXPECT_FALSE(character.is_dead());
}

TEST_F(CharacterTest, ReceiveDamageKillsCharacter) {
    character.receive_damage(100);
    EXPECT_EQ(character.get_current_hp(), 0);
    EXPECT_TRUE(character.is_dead());
}

TEST_F(CharacterTest, ReceiveDamageMoreThanHP) {
    character.receive_damage(150);
    EXPECT_EQ(character.get_current_hp(), 0);
    EXPECT_TRUE(character.is_dead());
}

TEST_F(CharacterTest, ReceiveDamageWhenAlreadyDead) {
    character.receive_damage(100);
    EXPECT_TRUE(character.is_dead());

    character.receive_damage(50);
    EXPECT_EQ(character.get_current_hp(), 0);
    EXPECT_TRUE(character.is_dead());
}

TEST_F(CharacterTest, ReceiveNegativeDamageIgnored) {
    character.receive_damage(-10);
    EXPECT_EQ(character.get_current_hp(), 100);
}

TEST_F(CharacterTest, ReceiveZeroDamageIgnored) {
    character.receive_damage(0);
    EXPECT_EQ(character.get_current_hp(), 100);
}

TEST_F(CharacterTest, HealRestoresHP) {
    character.receive_damage(30);
    EXPECT_EQ(character.get_current_hp(), 70);

    character.heal(15);
    EXPECT_EQ(character.get_current_hp(), 85);
}

TEST_F(CharacterTest, HealCannotExceedMaxHP) {
    character.heal(50);
    EXPECT_EQ(character.get_current_hp(), 100);
}

TEST_F(CharacterTest, HealWhenDead) {
    character.receive_damage(100);
    EXPECT_TRUE(character.is_dead());

    character.heal(50);
    EXPECT_EQ(character.get_current_hp(), 0);
    EXPECT_TRUE(character.is_dead());
}

TEST_F(CharacterTest, HealNegativeIgnored) {
    character.heal(-10);
    EXPECT_EQ(character.get_current_hp(), 100);
}

TEST_F(CharacterTest, SetPosition) {
    Position new_pos{30, 40};
    character.set_position(new_pos);

    const Position& pos = character.get_position();
    EXPECT_EQ(pos.x, 30);
    EXPECT_EQ(pos.y, 40);
}
