#include "gtest/gtest.h"
#include "server/world/zone.h"
#include "server/world/city.h"
#include "server/world/dungeon.h"

class CityTest : public ::testing::Test {
protected:
    City city;
};

class DungeonTest : public ::testing::Test {
protected:
    Dungeon dungeon;
};

class ZonePolymorphismTest : public ::testing::Test {
protected:
    Zone* city_zone = new City();
    Zone* dungeon_zone = new Dungeon();
};

TEST_F(CityTest, CityIsSafe) {
    EXPECT_TRUE(city.is_safe());
}

TEST_F(CityTest, CityCanSpawn) {
    EXPECT_TRUE(city.can_spawn());
}

TEST_F(DungeonTest, DungeonIsNotSafe) {
    EXPECT_FALSE(dungeon.is_safe());
}

TEST_F(DungeonTest, DungeonCanSpawn) {
    EXPECT_TRUE(dungeon.can_spawn());
}

TEST_F(ZonePolymorphismTest, CityPolymorphism) {
    EXPECT_TRUE(city_zone->is_safe());
    EXPECT_TRUE(city_zone->can_spawn());
}

TEST_F(ZonePolymorphismTest, DungeonPolymorphism) {
    EXPECT_FALSE(dungeon_zone->is_safe());
    EXPECT_TRUE(dungeon_zone->can_spawn());
}

TEST_F(ZonePolymorphismTest, VirtualDestructor) {
    delete city_zone;
    delete dungeon_zone;
    // ver que funciona el destructor virtual 
}

TEST(ZoneTest, SafetyDifference) {
    City city;
    Dungeon dungeon;
    
    EXPECT_NE(city.is_safe(), dungeon.is_safe());
}

TEST(ZoneTest, BothCanSpawn) {
    City city;
    Dungeon dungeon;
    
    EXPECT_TRUE(city.can_spawn());
    EXPECT_TRUE(dungeon.can_spawn());
}
