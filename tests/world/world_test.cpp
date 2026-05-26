#include "server/world/world.h"

#include "common/direction.h"
#include "common/position.h"
#include "gtest/gtest.h"
#include "server/game/player.h"
#include "server/game/player_class.h"
#include "server/game/player_race.h"

class WorldTest: public ::testing::Test {
 protected:
    World world{100, 80};

    std::unique_ptr<Player> create_player(uint32_t id, int x, int y) {
        return std::make_unique<Player>(id, "TestPlayer", PlayerRace::HUMAN, PlayerClass::WARRIOR,
                                        1, 100, 50, 10, 10, 10, 10, Position{x, y});
    }
};

TEST_F(WorldTest, AddPlayerBasic) {
    auto player = create_player(1, 50, 40);
    auto player_id = player->get_id();
    world.add_player(std::move(player));

    EXPECT_TRUE(world.player_exists(player_id));
    EXPECT_NE(world.get_player(player_id), nullptr);
}

TEST_F(WorldTest, AddPlayerMultiple) {
    world.add_player(create_player(1, 50, 40));
    world.add_player(create_player(2, 51, 41));
    world.add_player(create_player(3, 52, 42));

    EXPECT_TRUE(world.player_exists(1));
    EXPECT_TRUE(world.player_exists(2));
    EXPECT_TRUE(world.player_exists(3));
}

TEST_F(WorldTest, RemovePlayerBasic) {
    world.add_player(create_player(1, 50, 40));
    EXPECT_TRUE(world.player_exists(1));

    world.remove_player(1);
    EXPECT_FALSE(world.player_exists(1));
    EXPECT_EQ(world.get_player(1), nullptr);
}

TEST_F(WorldTest, RemovePlayerMultiple) {
    world.add_player(create_player(1, 50, 40));
    world.add_player(create_player(2, 51, 41));
    world.add_player(create_player(3, 52, 42));

    world.remove_player(2);

    EXPECT_TRUE(world.player_exists(1));
    EXPECT_FALSE(world.player_exists(2));
    EXPECT_TRUE(world.player_exists(3));
}

TEST_F(WorldTest, GetPlayerExistent) {
    world.add_player(create_player(1, 50, 40));
    Player* player1 = world.get_player(1);

    EXPECT_NE(player1, nullptr);
    EXPECT_EQ(player1->get_id(), 1);
}

TEST_F(WorldTest, GetPlayerNonExistent) {
    EXPECT_EQ(world.get_player(999), nullptr);
}

TEST_F(WorldTest, PlayerExistsTrue) {
    world.add_player(create_player(1, 50, 40));
    EXPECT_TRUE(world.player_exists(1));
}

TEST_F(WorldTest, PlayerExistsFalse) {
    EXPECT_FALSE(world.player_exists(1));
    EXPECT_FALSE(world.player_exists(999));
}

TEST_F(WorldTest, MovePlayerUp) {
    world.add_player(create_player(1, 50, 50));
    auto update = world.move_player(1, Direction::UP);

    EXPECT_NE(update, nullptr);
    EXPECT_EQ(world.get_player(1)->get_position().x, 50);
    EXPECT_EQ(world.get_player(1)->get_position().y, 49);
}

TEST_F(WorldTest, MovePlayerDown) {
    world.add_player(create_player(1, 50, 50));
    auto update = world.move_player(1, Direction::DOWN);

    EXPECT_NE(update, nullptr);
    EXPECT_EQ(world.get_player(1)->get_position().x, 50);
    EXPECT_EQ(world.get_player(1)->get_position().y, 51);
}

TEST_F(WorldTest, MovePlayerLeft) {
    world.add_player(create_player(1, 50, 50));
    auto update = world.move_player(1, Direction::LEFT);

    EXPECT_NE(update, nullptr);
    EXPECT_EQ(world.get_player(1)->get_position().x, 49);
    EXPECT_EQ(world.get_player(1)->get_position().y, 50);
}

TEST_F(WorldTest, MovePlayerRight) {
    world.add_player(create_player(1, 50, 50));
    auto update = world.move_player(1, Direction::RIGHT);

    EXPECT_NE(update, nullptr);
    EXPECT_EQ(world.get_player(1)->get_position().x, 51);
    EXPECT_EQ(world.get_player(1)->get_position().y, 50);
}

TEST_F(WorldTest, MovePlayerMultipleTimes) {
    world.add_player(create_player(1, 50, 50));
    world.move_player(1, Direction::UP);
    world.move_player(1, Direction::RIGHT);
    world.move_player(1, Direction::DOWN);
    world.move_player(1, Direction::RIGHT);

    EXPECT_EQ(world.get_player(1)->get_position().x, 52);
    EXPECT_EQ(world.get_player(1)->get_position().y, 50);
}

TEST_F(WorldTest, MovePlayerBoundOutOfBoundsUp) {
    world.add_player(create_player(1, 50, 0));
    auto update = world.move_player(1, Direction::UP);

    EXPECT_EQ(update, nullptr);
    auto pos = world.get_player(1)->get_position();
    EXPECT_EQ(pos.x, 50);
    EXPECT_EQ(pos.y, 0);
}

TEST_F(WorldTest, MovePlayerBoundOutOfBoundsDown) {
    world.add_player(create_player(1, 50, 79));  // valido de 0 a 79
    auto update = world.move_player(1, Direction::DOWN);

    EXPECT_EQ(update, nullptr);
    auto pos = world.get_player(1)->get_position();
    EXPECT_EQ(pos.x, 50);
    EXPECT_EQ(pos.y, 79);
}

TEST_F(WorldTest, MovePlayerBoundOutOfBoundsLeft) {
    world.add_player(create_player(1, 0, 50));
    auto update = world.move_player(1, Direction::LEFT);

    EXPECT_EQ(update, nullptr);
    auto pos = world.get_player(1)->get_position();
    EXPECT_EQ(pos.x, 0);
    EXPECT_EQ(pos.y, 50);
}

TEST_F(WorldTest, MovePlayerBoundOutOfBoundsRight) {
    world.add_player(create_player(1, 99, 50));  // valido de 0 a 99
    auto update = world.move_player(1, Direction::RIGHT);

    EXPECT_EQ(update, nullptr);
    auto pos = world.get_player(1)->get_position();
    EXPECT_EQ(pos.x, 99);
    EXPECT_EQ(pos.y, 50);
}

TEST_F(WorldTest, MovePlayerAtAllCorners) {
    world.add_player(create_player(1, 0, 0));
    EXPECT_EQ(world.move_player(1, Direction::UP), nullptr);
    EXPECT_EQ(world.move_player(1, Direction::LEFT), nullptr);

    world.remove_player(1);
    world.add_player(create_player(2, 99, 79));
    EXPECT_EQ(world.move_player(2, Direction::DOWN), nullptr);
    EXPECT_EQ(world.move_player(2, Direction::RIGHT), nullptr);
}

TEST_F(WorldTest, MovePlayerBlockingTerrain) {
    world.add_player(create_player(1, 50, 50));
    Cell blocking(TerrainType::WATER, true);
    world.set_cell(Position{50, 51}, blocking);

    auto update = world.move_player(1, Direction::DOWN);

    EXPECT_EQ(update, nullptr);
    auto pos = world.get_player(1)->get_position();
    EXPECT_EQ(pos.x, 50);
    EXPECT_EQ(pos.y, 50);
}

TEST_F(WorldTest, MovePlayerNonBlockingTerrain) {
    world.add_player(create_player(1, 50, 50));
    Cell grass(TerrainType::GRASS, false);
    world.set_cell(Position{50, 51}, grass);

    auto update = world.move_player(1, Direction::DOWN);

    EXPECT_NE(update, nullptr);
    EXPECT_EQ(world.get_player(1)->get_position().x, 50);
    EXPECT_EQ(world.get_player(1)->get_position().y, 51);
}

TEST_F(WorldTest, MovePlayerCollisionWithAnotherPlayer) {
    world.add_player(create_player(1, 50, 50));
    world.add_player(create_player(2, 50, 51));

    auto update = world.move_player(1, Direction::DOWN);

    EXPECT_EQ(update, nullptr);
    auto pos = world.get_player(1)->get_position();
    EXPECT_EQ(pos.x, 50);
    EXPECT_EQ(pos.y, 50);
}

TEST_F(WorldTest, MovePlayerNoCollisionWhenPathClear) {
    world.add_player(create_player(1, 50, 50));
    world.add_player(create_player(2, 50, 52));

    auto update = world.move_player(1, Direction::DOWN);

    EXPECT_NE(update, nullptr);
    auto pos = world.get_player(1)->get_position();
    EXPECT_EQ(pos.x, 50);
    EXPECT_EQ(pos.y, 51);
}

TEST_F(WorldTest, MovePlayerSurrounded) {
    world.add_player(create_player(1, 50, 50));
    world.add_player(create_player(2, 49, 50));
    world.add_player(create_player(3, 51, 50));
    world.add_player(create_player(4, 50, 49));
    world.add_player(create_player(5, 50, 51));

    EXPECT_EQ(world.move_player(1, Direction::UP), nullptr);
    EXPECT_EQ(world.move_player(1, Direction::DOWN), nullptr);
    EXPECT_EQ(world.move_player(1, Direction::LEFT), nullptr);
    EXPECT_EQ(world.move_player(1, Direction::RIGHT), nullptr);
    auto pos = world.get_player(1)->get_position();
    EXPECT_EQ(pos.x, 50);
    EXPECT_EQ(pos.y, 50);
}

TEST_F(WorldTest, MovePlayerCollisionAfterRemoval) {
    world.add_player(create_player(1, 50, 50));
    world.add_player(create_player(2, 50, 51));

    world.remove_player(2);
    auto update = world.move_player(1, Direction::DOWN);

    EXPECT_NE(update, nullptr);
    auto pos = world.get_player(1)->get_position();
    EXPECT_EQ(pos.x, 50);
    EXPECT_EQ(pos.y, 51);
}

TEST_F(WorldTest, MovePlayerNonExistent) {
    auto update = world.move_player(999, Direction::UP);
    EXPECT_EQ(update, nullptr);
}

TEST_F(WorldTest, MovePlayerCombinedBoundsAndTerrain) {
    world.add_player(create_player(1, 50, 0));
    Cell blocking(TerrainType::WATER, true);
    world.set_cell(Position{50, 1}, blocking);

    auto update = world.move_player(1, Direction::DOWN);
    EXPECT_EQ(update, nullptr);
}

TEST_F(WorldTest, MovePlayerCombinedTerrainAndCollision) {
    world.add_player(create_player(1, 50, 50));
    world.add_player(create_player(2, 51, 50));
    Cell blocking(TerrainType::STONE, true);
    world.set_cell(Position{49, 50}, blocking);

    auto update = world.move_player(1, Direction::LEFT);
    EXPECT_EQ(update, nullptr);
}

TEST_F(WorldTest, PositionMapConsistencyMultipleMoves) {
    world.add_player(create_player(1, 50, 50));
    world.add_player(create_player(2, 60, 60));

    world.move_player(1, Direction::UP);
    world.move_player(2, Direction::DOWN);
    world.move_player(1, Direction::LEFT);
    world.move_player(2, Direction::RIGHT);

    auto pos1 = world.get_player(1)->get_position();
    auto pos2 = world.get_player(2)->get_position();
    EXPECT_EQ(pos1.x, 49);
    EXPECT_EQ(pos1.y, 49);
    EXPECT_EQ(pos2.x, 61);
    EXPECT_EQ(pos2.y, 61);
}

TEST_F(WorldTest, LargeWorld) {
    World large(1000, 800);
    large.add_player(create_player(1, 999, 799));

    EXPECT_EQ(large.move_player(1, Direction::DOWN), nullptr);
    EXPECT_EQ(large.move_player(1, Direction::RIGHT), nullptr);
}

TEST_F(WorldTest, SmallWorldMovement) {
    World small(3, 3);
    small.add_player(create_player(1, 1, 1));

    EXPECT_NE(small.move_player(1, Direction::UP), nullptr);
    auto pos = small.get_player(1)->get_position();
    EXPECT_EQ(pos.x, 1);
    EXPECT_EQ(pos.y, 0);
}

TEST_F(WorldTest, MultiplePlayersIndependent) {
    world.add_player(create_player(1, 25, 25));
    world.add_player(create_player(2, 75, 75));

    world.move_player(1, Direction::UP);
    world.move_player(1, Direction::LEFT);
    world.move_player(2, Direction::DOWN);
    world.move_player(2, Direction::RIGHT);

    auto pos1 = world.get_player(1)->get_position();
    auto pos2 = world.get_player(2)->get_position();
    EXPECT_EQ(pos1.x, 24);
    EXPECT_EQ(pos1.y, 24);
    EXPECT_EQ(pos2.x, 76);
    EXPECT_EQ(pos2.y, 76);
}

TEST_F(WorldTest, PlayerRemovalDoesNotAffectOthers) {
    world.add_player(create_player(1, 50, 50));
    world.add_player(create_player(2, 50, 51));

    world.remove_player(1);
    auto update = world.move_player(2, Direction::UP);

    EXPECT_NE(update, nullptr);
    auto pos = world.get_player(2)->get_position();
    EXPECT_EQ(pos.x, 50);
    EXPECT_EQ(pos.y, 50);
}
