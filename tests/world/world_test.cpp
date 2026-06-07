#include "server/world/world.h"

#include "common/direction.h"
#include "common/position.h"
#include "gtest/gtest.h"
#include "server/game/game_config.h"
#include "server/game/items/weapon.h"
#include "server/game/loot.h"
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
    EXPECT_NO_THROW(world.move_player(1, Direction::UP));
    EXPECT_EQ(world.get_player(1)->get_position().x, 50);
    EXPECT_EQ(world.get_player(1)->get_position().y, 49);
}

TEST_F(WorldTest, MovePlayerDown) {
    world.add_player(create_player(1, 50, 50));
    EXPECT_NO_THROW(world.move_player(1, Direction::DOWN));
    EXPECT_EQ(world.get_player(1)->get_position().x, 50);
    EXPECT_EQ(world.get_player(1)->get_position().y, 51);
}

TEST_F(WorldTest, MovePlayerLeft) {
    world.add_player(create_player(1, 50, 50));
    EXPECT_NO_THROW(world.move_player(1, Direction::LEFT));
    EXPECT_EQ(world.get_player(1)->get_position().x, 49);
    EXPECT_EQ(world.get_player(1)->get_position().y, 50);
}

TEST_F(WorldTest, MovePlayerRight) {
    world.add_player(create_player(1, 50, 50));
    EXPECT_NO_THROW(world.move_player(1, Direction::RIGHT));
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

    EXPECT_THROW(world.move_player(1, Direction::UP), std::runtime_error);
    auto pos = world.get_player(1)->get_position();
    EXPECT_EQ(pos.x, 50);
    EXPECT_EQ(pos.y, 0);
}

TEST_F(WorldTest, MovePlayerBoundOutOfBoundsDown) {
    world.add_player(create_player(1, 50, 79));  // valido de 0 a 79

    EXPECT_THROW(world.move_player(1, Direction::DOWN), std::runtime_error);
    auto pos = world.get_player(1)->get_position();
    EXPECT_EQ(pos.x, 50);
    EXPECT_EQ(pos.y, 79);
}

TEST_F(WorldTest, MovePlayerBoundOutOfBoundsLeft) {
    world.add_player(create_player(1, 0, 50));

    EXPECT_THROW(world.move_player(1, Direction::LEFT), std::runtime_error);
    auto pos = world.get_player(1)->get_position();
    EXPECT_EQ(pos.x, 0);
    EXPECT_EQ(pos.y, 50);
}

TEST_F(WorldTest, MovePlayerBoundOutOfBoundsRight) {
    world.add_player(create_player(1, 99, 50));  // valido de 0 a 99

    EXPECT_THROW(world.move_player(1, Direction::RIGHT), std::runtime_error);
    auto pos = world.get_player(1)->get_position();
    EXPECT_EQ(pos.x, 99);
    EXPECT_EQ(pos.y, 50);
}

TEST_F(WorldTest, MovePlayerAtAllCorners) {
    world.add_player(create_player(1, 0, 0));
    EXPECT_THROW(world.move_player(1, Direction::UP), std::runtime_error);
    EXPECT_THROW(world.move_player(1, Direction::LEFT), std::runtime_error);

    world.remove_player(1);
    world.add_player(create_player(2, 99, 79));
    EXPECT_THROW(world.move_player(2, Direction::DOWN), std::runtime_error);
    EXPECT_THROW(world.move_player(2, Direction::RIGHT), std::runtime_error);
}

TEST_F(WorldTest, MovePlayerBlockingTerrain) {
    world.add_player(create_player(1, 50, 50));
    Cell blocking(TerrainType::WATER, true);
    world.set_cell(Position{50, 51}, blocking);

    EXPECT_THROW(world.move_player(1, Direction::DOWN), std::runtime_error);
    auto pos = world.get_player(1)->get_position();
    EXPECT_EQ(pos.x, 50);
    EXPECT_EQ(pos.y, 50);
}

TEST_F(WorldTest, MovePlayerNonBlockingTerrain) {
    world.add_player(create_player(1, 50, 50));
    Cell grass(TerrainType::GRASS, false);
    world.set_cell(Position{50, 51}, grass);

    EXPECT_NO_THROW(world.move_player(1, Direction::DOWN));
    EXPECT_EQ(world.get_player(1)->get_position().x, 50);
    EXPECT_EQ(world.get_player(1)->get_position().y, 51);
}

TEST_F(WorldTest, MovePlayerCollisionWithAnotherPlayer) {
    world.add_player(create_player(1, 50, 50));
    world.add_player(create_player(2, 50, 51));

    EXPECT_THROW(world.move_player(1, Direction::DOWN), std::runtime_error);
    auto pos = world.get_player(1)->get_position();
    EXPECT_EQ(pos.x, 50);
    EXPECT_EQ(pos.y, 50);
}

TEST_F(WorldTest, MovePlayerNoCollisionWhenPathClear) {
    world.add_player(create_player(1, 50, 50));
    world.add_player(create_player(2, 50, 52));

    EXPECT_NO_THROW(world.move_player(1, Direction::DOWN));
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

    EXPECT_THROW(world.move_player(1, Direction::UP), std::runtime_error);
    EXPECT_THROW(world.move_player(1, Direction::DOWN), std::runtime_error);
    EXPECT_THROW(world.move_player(1, Direction::LEFT), std::runtime_error);
    EXPECT_THROW(world.move_player(1, Direction::RIGHT), std::runtime_error);
    auto pos = world.get_player(1)->get_position();
    EXPECT_EQ(pos.x, 50);
    EXPECT_EQ(pos.y, 50);
}

TEST_F(WorldTest, MovePlayerCollisionAfterRemoval) {
    world.add_player(create_player(1, 50, 50));
    world.add_player(create_player(2, 50, 51));

    world.remove_player(2);
    EXPECT_NO_THROW(world.move_player(1, Direction::DOWN));
    auto pos = world.get_player(1)->get_position();
    EXPECT_EQ(pos.x, 50);
    EXPECT_EQ(pos.y, 51);
}

TEST_F(WorldTest, MovePlayerNonExistent) {
    EXPECT_THROW(world.move_player(999, Direction::UP), std::runtime_error);
}

TEST_F(WorldTest, MovePlayerCombinedBoundsAndTerrain) {
    world.add_player(create_player(1, 50, 0));
    Cell blocking(TerrainType::WATER, true);
    world.set_cell(Position{50, 1}, blocking);

    EXPECT_THROW(world.move_player(1, Direction::DOWN), std::runtime_error);
}

TEST_F(WorldTest, MovePlayerCombinedTerrainAndCollision) {
    world.add_player(create_player(1, 50, 50));
    world.add_player(create_player(2, 51, 50));
    Cell blocking(TerrainType::STONE, true);
    world.set_cell(Position{49, 50}, blocking);

    EXPECT_THROW(world.move_player(1, Direction::LEFT), std::runtime_error);
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

    EXPECT_THROW(large.move_player(1, Direction::DOWN), std::runtime_error);
    EXPECT_THROW(large.move_player(1, Direction::RIGHT), std::runtime_error);
}

TEST_F(WorldTest, SmallWorldMovement) {
    World small(3, 3);
    small.add_player(create_player(1, 1, 1));

    EXPECT_NO_THROW(small.move_player(1, Direction::UP));
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
    EXPECT_NO_THROW(world.move_player(2, Direction::UP));
    auto pos = world.get_player(2)->get_position();
    EXPECT_EQ(pos.x, 50);
    EXPECT_EQ(pos.y, 50);
}

TEST_F(WorldTest, PickUpGoldSuccess) {
    world.add_player(create_player(1, 50, 50));
    Player* player = world.get_player(1);
    uint64_t initial_gold = player->get_gold();

    Loot loot;
    loot.dropped_gold = 150;
    world.drop_loot_in_world(Position{50, 50}, std::move(loot));

    EXPECT_NO_THROW(world.pick_up_item(1));
    EXPECT_EQ(player->get_gold(), initial_gold + 150);
    EXPECT_TRUE(world.get_ground_items().empty());
}

TEST_F(WorldTest, PickUpItemSuccess) {
    world.add_player(create_player(1, 50, 50));
    Player* player = world.get_player(1);

    Loot loot;
    loot.dropped_items.push_back(
        InventorySlot{std::make_unique<Weapon>("GroundSword", 10, 20, false), 1, std::nullopt});
    world.drop_loot_in_world(Position{50, 50}, std::move(loot));

    EXPECT_EQ(player->get_inventory().get_size(), 0);
    EXPECT_NO_THROW(world.pick_up_item(1));
    EXPECT_EQ(player->get_inventory().get_size(), 1);
    EXPECT_TRUE(world.get_ground_items().empty());
}

TEST_F(WorldTest, PickUpItemInventoryFull) {
    world.add_player(create_player(1, 50, 50));
    Player* player = world.get_player(1);

    int max_items = GameConfig::get_instance().get_max_inventory_items();
    for (int i = 0; i < max_items; ++i) {
        player->get_inventory().add_item(std::make_unique<Weapon>("Sword", 10, 20, false));
    }

    Loot loot;
    loot.dropped_items.push_back(
        InventorySlot{std::make_unique<Weapon>("GroundSword", 10, 20, false), 1, std::nullopt});
    world.drop_loot_in_world(Position{50, 50}, std::move(loot));

    EXPECT_THROW(world.pick_up_item(1), std::runtime_error);
    EXPECT_EQ(player->get_inventory().get_size(), max_items);  // Inventario sigue lleno

    // El item debe seguir en el piso intacto (validando el arreglo de std::move)
    const auto& ground = world.get_ground_items();
    ASSERT_FALSE(ground.empty());
    EXPECT_NE(ground.at(Position{50, 50}).item, nullptr);
}

TEST_F(WorldTest, PickUpNothing) {
    world.add_player(create_player(1, 50, 50));
    EXPECT_THROW(world.pick_up_item(1), std::runtime_error);
}

TEST_F(WorldTest, DropItemSuccess) {
    world.add_player(create_player(1, 50, 50));
    Player* player = world.get_player(1);
    player->get_inventory().add_item(std::make_unique<Weapon>("Sword", 10, 20, false));

    EXPECT_EQ(player->get_inventory().get_size(), 1);
    EXPECT_TRUE(world.get_ground_items().empty());

    EXPECT_NO_THROW(world.drop_item(1, 0));  // Tiramos el slot 0

    EXPECT_EQ(player->get_inventory().get_size(), 0);

    const auto& ground = world.get_ground_items();
    ASSERT_FALSE(ground.empty());

    // Verifica que esté en la posición (nuestro algoritmo BFS lo deja ahí mismo si está libre)
    auto it = ground.find(Position{50, 50});
    ASSERT_NE(it, ground.end());
    EXPECT_EQ(it->second.item->get_name(), "Sword");
}

TEST_F(WorldTest, DropItemInvalidSlot) {
    world.add_player(create_player(1, 50, 50));
    EXPECT_THROW(world.drop_item(1, 0), std::runtime_error);
}

TEST_F(WorldTest, DeadPlayerCannotPickUpOrDrop) {
    world.add_player(create_player(1, 50, 50));
    Player* player = world.get_player(1);

    player->get_inventory().add_item(std::make_unique<Weapon>("Sword", 10, 20, false));
    player->receive_damage(9999);
    EXPECT_TRUE(player->is_dead());

    Loot loot;
    loot.dropped_gold = 100;
    world.drop_loot_in_world(Position{50, 50}, std::move(loot));

    EXPECT_THROW(world.pick_up_item(1), std::runtime_error);
    EXPECT_THROW(world.drop_item(1, 0), std::runtime_error);
}
