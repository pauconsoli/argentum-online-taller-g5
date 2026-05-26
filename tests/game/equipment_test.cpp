#include "server/game/equipment.h"

#include "gtest/gtest.h"
#include "server/game/items/defensive_item.h"
#include "server/game/items/magic_weapon.h"
#include "server/game/items/normal_weapon.h"
#include "server/game/items/spell.h"

class EquipmentTest: public ::testing::Test {
 protected:
    Equipment equipment;

    NormalWeapon sword{"Espada", 5, 15, false};
    NormalWeapon axe{"Hacha", 8, 20, false};

    Spell fireball{"Fireball", 20, 40, 0, 0};
    MagicWeapon staff{"Báculo", std::make_unique<Spell>(fireball), 30};

    DefensiveItem armor{"Armadura de Hierro", 5, 10, EquipmentSlot::ARMOR};
    DefensiveItem helmet{"Casco", 3, 6, EquipmentSlot::HELMET};
    DefensiveItem shield{"Escudo", 4, 8, EquipmentSlot::SHIELD};
};


TEST_F(EquipmentTest, EquipWeaponInEmptySlot) {
    Item* prev = equipment.equip(&sword, EquipmentSlot::WEAPON);
    EXPECT_EQ(prev, nullptr);  // no había nada antes
    EXPECT_EQ(equipment.get_item_from_slot(EquipmentSlot::WEAPON), &sword);
}

TEST_F(EquipmentTest, EquipArmorInEmptySlot) {
    Item* prev = equipment.equip(&armor, EquipmentSlot::ARMOR);
    EXPECT_EQ(prev, nullptr);
    EXPECT_EQ(equipment.get_item_from_slot(EquipmentSlot::ARMOR), &armor);
}

TEST_F(EquipmentTest, SwapWeaponForMagicWeapon) {
    // equipar espada normal primero
    equipment.equip(&sword, EquipmentSlot::WEAPON);
    EXPECT_EQ(equipment.get_item_from_slot(EquipmentSlot::WEAPON), &sword);

    // equipar báculo (reemplaza la espada, no pueden estar ambos)
    Item* prev = equipment.equip(&staff, EquipmentSlot::WEAPON);
    EXPECT_EQ(prev, &sword);  // devuelve la espada anterior
    EXPECT_EQ(equipment.get_item_from_slot(EquipmentSlot::WEAPON), &staff);  // ahora es el báculo
}

TEST_F(EquipmentTest, SwapMultipleItems) {
    equipment.equip(&sword, EquipmentSlot::WEAPON);
    equipment.equip(&armor, EquipmentSlot::ARMOR);
    equipment.equip(&helmet, EquipmentSlot::HELMET);

    Item* prev_weapon = equipment.equip(&axe, EquipmentSlot::WEAPON);
    EXPECT_EQ(prev_weapon, &sword);

    DefensiveItem better_armor{"Armadura Legendaria", 10, 20, EquipmentSlot::ARMOR};
    Item* prev_armor = equipment.equip(&better_armor, EquipmentSlot::ARMOR);
    EXPECT_EQ(prev_armor, &armor);

    EXPECT_EQ(equipment.get_item_from_slot(EquipmentSlot::WEAPON), &axe);
    EXPECT_EQ(equipment.get_item_from_slot(EquipmentSlot::ARMOR), &better_armor);
    EXPECT_EQ(equipment.get_item_from_slot(EquipmentSlot::HELMET), &helmet);
}


TEST_F(EquipmentTest, UnequipFromEmptySlot) {
    Item* item = equipment.unequip(EquipmentSlot::WEAPON);
    EXPECT_EQ(item, nullptr);
}

TEST_F(EquipmentTest, UnequipEquippedItem) {
    equipment.equip(&sword, EquipmentSlot::WEAPON);
    Item* item = equipment.unequip(EquipmentSlot::WEAPON);
    EXPECT_EQ(item, &sword);
    EXPECT_EQ(equipment.get_item_from_slot(EquipmentSlot::WEAPON), nullptr);
}

TEST_F(EquipmentTest, UnequipMultipleSlots) {
    equipment.equip(&sword, EquipmentSlot::WEAPON);
    equipment.equip(&armor, EquipmentSlot::ARMOR);
    equipment.equip(&helmet, EquipmentSlot::HELMET);
    equipment.equip(&shield, EquipmentSlot::SHIELD);

    EXPECT_EQ(equipment.unequip(EquipmentSlot::HELMET), &helmet);
    EXPECT_EQ(equipment.unequip(EquipmentSlot::SHIELD), &shield);

    EXPECT_EQ(equipment.get_item_from_slot(EquipmentSlot::WEAPON), &sword);
    EXPECT_EQ(equipment.get_item_from_slot(EquipmentSlot::ARMOR), &armor);
    EXPECT_EQ(equipment.get_item_from_slot(EquipmentSlot::HELMET), nullptr);
    EXPECT_EQ(equipment.get_item_from_slot(EquipmentSlot::SHIELD), nullptr);
}


TEST_F(EquipmentTest, HasEquippedItem) {
    EXPECT_FALSE(equipment.slot_has_item(EquipmentSlot::WEAPON));
    equipment.equip(&sword, EquipmentSlot::WEAPON);
    EXPECT_TRUE(equipment.slot_has_item(EquipmentSlot::WEAPON));
}

TEST_F(EquipmentTest, HasAfterUnequip) {
    equipment.equip(&sword, EquipmentSlot::WEAPON);
    EXPECT_TRUE(equipment.slot_has_item(EquipmentSlot::WEAPON));
    equipment.unequip(EquipmentSlot::WEAPON);
    EXPECT_FALSE(equipment.slot_has_item(EquipmentSlot::WEAPON));
}

TEST_F(EquipmentTest, GetFromEmptySlot) {
    EXPECT_EQ(equipment.get_item_from_slot(EquipmentSlot::WEAPON), nullptr);
}

TEST_F(EquipmentTest, GetAllSlots) {
    equipment.equip(&sword, EquipmentSlot::WEAPON);
    equipment.equip(&armor, EquipmentSlot::ARMOR);
    equipment.equip(&helmet, EquipmentSlot::HELMET);
    equipment.equip(&shield, EquipmentSlot::SHIELD);

    EXPECT_EQ(equipment.get_item_from_slot(EquipmentSlot::WEAPON), &sword);
    EXPECT_EQ(equipment.get_item_from_slot(EquipmentSlot::ARMOR), &armor);
    EXPECT_EQ(equipment.get_item_from_slot(EquipmentSlot::HELMET), &helmet);
    EXPECT_EQ(equipment.get_item_from_slot(EquipmentSlot::SHIELD), &shield);
}
