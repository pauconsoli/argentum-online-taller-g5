#include <memory>
#include <vector>

#include "common/position.h"
#include "gtest/gtest.h"
#include "server/game/npcs/banker.h"
#include "server/game/npcs/city_npc.h"
#include "server/game/npcs/hostile_npc.h"
#include "server/game/npcs/merchant.h"
#include "server/game/npcs/priest.h"
#include "server/game/player.h"

class NPCTest: public ::testing::Test {
 protected:
    Position pos{10, 10};
    std::vector<std::string> zones{"all"};

    HostileNPC goblin{1, "Goblin", 3, 40, 2, 10, 3, 8, 5, zones, pos};

    // ADYACENTE
    std::unique_ptr<Player> p1 =
        std::make_unique<Player>(100, "P1", PlayerRace::HUMAN, PlayerClass::WARRIOR, 1, 100, 0, 10,
                                 10, 10, 10, Position{10, 11});

    // EN RANGO PERO NO ADYACENTE
    std::unique_ptr<Player> p2 =
        std::make_unique<Player>(101, "P2", PlayerRace::HUMAN, PlayerClass::WARRIOR, 1, 100, 0, 10,
                                 10, 10, 10, Position{13, 14});

    // FUERA DE RANGO (MUY LEJOS)
    std::unique_ptr<Player> p3 =
        std::make_unique<Player>(102, "P3", PlayerRace::HUMAN, PlayerClass::WARRIOR, 1, 100, 0, 10,
                                 10, 10, 10, Position{20, 20});
};

TEST_F(NPCTest, InitializationHostile) {
    EXPECT_EQ(goblin.get_name(), "Goblin");
    EXPECT_EQ(goblin.get_level(), 3);
    EXPECT_EQ(goblin.get_max_hp(), 40);
    EXPECT_EQ(goblin.get_defense(), 2);
    EXPECT_EQ(goblin.get_agility(), 10);
    EXPECT_TRUE(goblin.is_hostile());
}

TEST_F(NPCTest, UpdateEmptyTargetsReturnsStill) {
    auto behavior = goblin.update(1.0f, {});
    EXPECT_EQ(behavior.action, NPCAction::STILL);
}

TEST_F(NPCTest, UpdateAdjacentTargetReturnsAttack) {
    std::vector<Player*> targets = {p1.get()};
    auto behavior = goblin.update(1.0f, targets);

    EXPECT_EQ(behavior.action, NPCAction::ATTACK);
    EXPECT_EQ(behavior.target_id, 100u);
}

TEST_F(NPCTest, UpdateTargetInRangeReturnsChase) {
    std::vector<Player*> targets = {p2.get()};
    auto behavior = goblin.update(1.0f, targets);

    EXPECT_EQ(behavior.action, NPCAction::CHASE);
    EXPECT_EQ(behavior.target_position.x, 13);
    EXPECT_EQ(behavior.target_position.y, 14);
}

TEST_F(NPCTest, UpdateTargetOutOfRangeReturnsStill) {
    std::vector<Player*> targets = {p3.get()};
    auto behavior = goblin.update(1.0f, targets);
    EXPECT_EQ(behavior.action, NPCAction::STILL);
}

TEST_F(NPCTest, UpdateIgnoresDeadTargets) {
    p1->receive_damage(9999);  // Matamos al jugador adyacente
    EXPECT_TRUE(p1->is_dead());

    std::vector<Player*> targets = {p1.get()};
    auto behavior = goblin.update(1.0f, targets);
    EXPECT_EQ(behavior.action, NPCAction::STILL);
}
