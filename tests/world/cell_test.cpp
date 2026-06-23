#include "server/world/cell.h"

#include "common/world/terrain_type.h"
#include "gtest/gtest.h"

class CellTest: public ::testing::Test {
 protected:
    Cell grass_cell{TerrainType::GRASS, false};
    Cell grass_blocking_cell{TerrainType::GRASS, true};
    Cell water_cell{TerrainType::WATER, true};
    Cell stone_cell{TerrainType::STONE, false};
};

TEST_F(CellTest, DefaultConstructor) {
    Cell default_cell;
    EXPECT_EQ(default_cell.get_terrain_type(), TerrainType::GRASS);
    EXPECT_FALSE(default_cell.is_blocking());
}

TEST_F(CellTest, ConstructorWithTerrainOnly) {
    Cell dirt_cell(TerrainType::DIRT);
    EXPECT_EQ(dirt_cell.get_terrain_type(), TerrainType::DIRT);
    EXPECT_FALSE(dirt_cell.is_blocking());
}

TEST_F(CellTest, ConstructorWithBlockingGrass) {
    EXPECT_EQ(grass_blocking_cell.get_terrain_type(), TerrainType::GRASS);
    EXPECT_TRUE(grass_blocking_cell.is_blocking());
}

TEST_F(CellTest, WaterIsBlocking) {
    EXPECT_EQ(water_cell.get_terrain_type(), TerrainType::WATER);
    EXPECT_TRUE(water_cell.is_blocking());
}

TEST_F(CellTest, StoneIsNotBlocking) {
    EXPECT_EQ(stone_cell.get_terrain_type(), TerrainType::STONE);
    EXPECT_FALSE(stone_cell.is_blocking());
}

TEST_F(CellTest, AllTerrainTypes) {
    Cell grass(TerrainType::GRASS, false);
    Cell water(TerrainType::WATER, false);
    Cell dirt(TerrainType::DIRT, false);
    Cell stone(TerrainType::STONE, false);
    Cell sand(TerrainType::SAND, false);

    EXPECT_EQ(grass.get_terrain_type(), TerrainType::GRASS);
    EXPECT_EQ(water.get_terrain_type(), TerrainType::WATER);
    EXPECT_EQ(dirt.get_terrain_type(), TerrainType::DIRT);
    EXPECT_EQ(stone.get_terrain_type(), TerrainType::STONE);
    EXPECT_EQ(sand.get_terrain_type(), TerrainType::SAND);
}
