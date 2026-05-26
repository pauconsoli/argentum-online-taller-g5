#include "server/world/world_map.h"

#include "common/position.h"
#include "gtest/gtest.h"
#include "server/world/city.h"
#include "server/world/dungeon.h"

class WorldMapTest: public ::testing::Test {
 protected:
    WorldMap map{100, 80};
    Position center{50, 40};
    Position edge{0, 0};
    Position corner{99, 79};
    Position out_of_bounds{100, 100};
    Position negative{-1, -1};

    City city_zone;
    Dungeon dungeon_zone;
};

TEST_F(WorldMapTest, ConstructorDimensions) {
    EXPECT_EQ(map.get_width(), 100);
    EXPECT_EQ(map.get_height(), 80);
}

TEST_F(WorldMapTest, IsValidPositionCenter) {
    EXPECT_TRUE(map.is_valid_position(center));
}

TEST_F(WorldMapTest, IsValidPositionEdges) {
    EXPECT_TRUE(map.is_valid_position(edge));
    EXPECT_TRUE(map.is_valid_position(corner));
}

TEST_F(WorldMapTest, IsValidPositionOutOfBounds) {
    EXPECT_FALSE(map.is_valid_position(out_of_bounds));
    EXPECT_FALSE(map.is_valid_position(negative));
}

TEST_F(WorldMapTest, IsValidPositionBoundary) {
    EXPECT_TRUE(map.is_valid_position(Position{99, 0}));
    EXPECT_TRUE(map.is_valid_position(Position{0, 79}));
    EXPECT_FALSE(map.is_valid_position(Position{100, 0}));
    EXPECT_FALSE(map.is_valid_position(Position{0, 80}));
}

TEST_F(WorldMapTest, IsBlockingValidPosition) {
    EXPECT_FALSE(map.is_position_blocked(center));  // por defecto las celdas no son bloqueantes
}

TEST_F(WorldMapTest, IsBlockingOutOfBounds) {
    // fuera del mapa siempre bloqueante
    EXPECT_TRUE(map.is_position_blocked(out_of_bounds));
    EXPECT_TRUE(map.is_position_blocked(negative));
}

TEST_F(WorldMapTest, SetCellAndGetCell) {
    Cell grass(TerrainType::GRASS, false);
    map.set_cell(center, grass);

    const Cell& retrieved = map.get_cell(center);
    EXPECT_EQ(retrieved.get_terrain_type(), TerrainType::GRASS);
    EXPECT_FALSE(retrieved.is_blocking());
}

TEST_F(WorldMapTest, SetBlockingCell) {
    Cell water(TerrainType::WATER, true);
    map.set_cell(center, water);

    EXPECT_TRUE(map.is_position_blocked(center));
}

TEST_F(WorldMapTest, SetZoneCity) {
    map.set_zone(center, &city_zone);

    Zone* retrieved = map.get_zone(center);
    EXPECT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved, &city_zone);
    EXPECT_TRUE(retrieved->is_safe());
}

TEST_F(WorldMapTest, SetZoneDungeon) {
    map.set_zone(center, &dungeon_zone);

    Zone* retrieved = map.get_zone(center);
    EXPECT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved, &dungeon_zone);
    EXPECT_FALSE(retrieved->is_safe());
}

TEST_F(WorldMapTest, GetZoneOutOfBounds) {
    Zone* zone = map.get_zone(out_of_bounds);
    EXPECT_EQ(zone, nullptr);
}

TEST_F(WorldMapTest, IsSafeWithCity) {
    map.set_zone(center, &city_zone);
    EXPECT_TRUE(map.is_safe(center));
}

TEST_F(WorldMapTest, IsSafeWithDungeon) {
    map.set_zone(center, &dungeon_zone);
    EXPECT_FALSE(map.is_safe(center));
}

TEST_F(WorldMapTest, IsSafeWithoutZone) {
    EXPECT_FALSE(map.is_safe(center));  // sin zona, no es seguro
}

TEST_F(WorldMapTest, SetZoneOutOfBoundsIgnored) {
    map.set_zone(out_of_bounds, &city_zone);

    Zone* zone = map.get_zone(out_of_bounds);
    EXPECT_EQ(zone, nullptr);  // aún no hay zona porque la posición es inválida
}

TEST_F(WorldMapTest, SetCellOutOfBoundsIgnored) {
    Cell water(TerrainType::WATER, true);
    map.set_cell(out_of_bounds, water);

    bool is_blocking = map.is_position_blocked(out_of_bounds);
    EXPECT_TRUE(is_blocking);  // verdadero porque fuera del mapa es bloqueante
}

TEST_F(WorldMapTest, MultipleZones) {
    Dungeon dungeon2;

    map.set_zone(center, &city_zone);
    map.set_zone(Position{10, 10}, &dungeon2);

    EXPECT_TRUE(map.is_safe(center));
    EXPECT_FALSE(map.is_safe(Position{10, 10}));
}

TEST_F(WorldMapTest, OverwriteZone) {
    map.set_zone(center, &city_zone);
    EXPECT_TRUE(map.is_safe(center));

    map.set_zone(center, &dungeon_zone);
    EXPECT_FALSE(map.is_safe(center));
}

TEST_F(WorldMapTest, SmallMap) {
    WorldMap small_map(10, 10);

    EXPECT_EQ(small_map.get_width(), 10);
    EXPECT_EQ(small_map.get_height(), 10);
    EXPECT_TRUE(small_map.is_valid_position(Position{9, 9}));
    EXPECT_FALSE(small_map.is_valid_position(Position{10, 10}));
}

TEST_F(WorldMapTest, LargeMap) {
    WorldMap large_map(5000, 5000);

    EXPECT_EQ(large_map.get_width(), 5000);
    EXPECT_EQ(large_map.get_height(), 5000);
    EXPECT_TRUE(large_map.is_valid_position(Position{4999, 4999}));
    EXPECT_FALSE(large_map.is_valid_position(Position{5000, 5000}));
}
