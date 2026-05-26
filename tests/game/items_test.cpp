#include "gtest/gtest.h"
#include "server/game/items/consumable_item.h"
#include "server/game/items/defensive_item.h"
#include "server/game/items/item.h"
#include "server/game/items/magic_weapon.h"
#include "server/game/items/normal_weapon.h"
#include "server/game/items/spell.h"
#include "server/game/items/weapon.h"

class SpellTest: public ::testing::Test {
 protected:
    Spell damage_spell{"Fireball", 20, 40, 0, 0};
    Spell heal_spell{"Heal", 0, 0, 30, 50};
};

TEST_F(SpellTest, SpellNameInitialization) {
    EXPECT_EQ(damage_spell.get_name(), "Fireball");
    EXPECT_EQ(heal_spell.get_name(), "Heal");
}

TEST_F(SpellTest, SpellRange) {
    EXPECT_EQ(damage_spell.get_min_damage(), 20);
    EXPECT_EQ(damage_spell.get_max_damage(), 40);
    EXPECT_EQ(heal_spell.get_min_heal(), 30);
    EXPECT_EQ(heal_spell.get_max_heal(), 50);
}

TEST_F(SpellTest, SpellDoesDamage) {
    EXPECT_TRUE(damage_spell.does_damage());
    EXPECT_FALSE(heal_spell.does_damage());
}

TEST_F(SpellTest, SpellDoesHealing) {
    EXPECT_FALSE(damage_spell.does_heal());
    EXPECT_TRUE(heal_spell.does_heal());
}

TEST_F(SpellTest, DamageSpell) {
    EXPECT_TRUE(damage_spell.does_damage());
    EXPECT_FALSE(damage_spell.does_heal());
    EXPECT_EQ(damage_spell.get_min_heal(), 0);
    EXPECT_EQ(damage_spell.get_max_heal(), 0);
}

TEST_F(SpellTest, HealSpell) {
    EXPECT_FALSE(heal_spell.does_damage());
    EXPECT_TRUE(heal_spell.does_heal());
    EXPECT_EQ(heal_spell.get_min_damage(), 0);
    EXPECT_EQ(heal_spell.get_max_damage(), 0);
}


class WeaponTest: public ::testing::Test {
 protected:
    Weapon sword{"Excalibur"};
    Weapon dagger{"Dagger"};
};

TEST_F(WeaponTest, WeaponInitialization) {
    EXPECT_EQ(sword.get_name(), "Excalibur");
    EXPECT_EQ(dagger.get_name(), "Dagger");
}

TEST_F(WeaponTest, WeaponEquipmentSlot) {
    auto slot = sword.get_slot();
    ASSERT_TRUE(slot.has_value());
    EXPECT_EQ(slot.value(), EquipmentSlot::WEAPON);
}


class NormalWeaponTest: public ::testing::Test {
 protected:
    NormalWeapon long_sword{"Long Sword", 15, 30, false};
    NormalWeapon bow{"Bow", 10, 20, true};
    NormalWeapon dagger{"Dagger", 5, 12, false};
};

TEST_F(NormalWeaponTest, NormalWeaponInitialization) {
    EXPECT_EQ(long_sword.get_name(), "Long Sword");
    EXPECT_EQ(bow.get_name(), "Bow");
}

TEST_F(NormalWeaponTest, NormalWeaponDamageRange) {
    EXPECT_EQ(long_sword.get_min_damage(), 15);
    EXPECT_EQ(long_sword.get_max_damage(), 30);
    EXPECT_EQ(dagger.get_min_damage(), 5);
    EXPECT_EQ(dagger.get_max_damage(), 12);
}

TEST_F(NormalWeaponTest, RangedWeapon) {
    EXPECT_TRUE(bow.is_ranged());
    EXPECT_FALSE(long_sword.is_ranged());
    EXPECT_FALSE(dagger.is_ranged());
}

TEST_F(NormalWeaponTest, MeleeWeapon) {
    EXPECT_FALSE(long_sword.is_ranged());
}

TEST_F(NormalWeaponTest, NormalWeaponIsEquippable) {
    auto slot = long_sword.get_slot();
    ASSERT_TRUE(slot.has_value());
    EXPECT_EQ(slot.value(), EquipmentSlot::WEAPON);
}


class MagicWeaponTest: public ::testing::Test {
 protected:
    std::unique_ptr<Spell> fireball{std::make_unique<Spell>("Fireball", 20, 40, 0, 0)};
    std::unique_ptr<Spell> heal{std::make_unique<Spell>("Heal", 0, 0, 30, 50)};

    MagicWeapon staff{"Staff of Fire Ruby", std::make_unique<Spell>("Fireball", 20, 40, 0, 0), 30};
    MagicWeapon healing_wand{"Wand of Healing", std::make_unique<Spell>("Heal", 0, 0, 30, 50), 20};
};

TEST_F(MagicWeaponTest, MagicWeaponInitialization) {
    EXPECT_EQ(staff.get_name(), "Staff of Fire Ruby");
    EXPECT_EQ(healing_wand.get_name(), "Wand of Healing");
}

TEST_F(MagicWeaponTest, MagicWeaponSpell) {
    const Spell& staff_spell = staff.get_spell();
    EXPECT_EQ(staff_spell.get_name(), "Fireball");
    EXPECT_EQ(staff_spell.get_min_damage(), 20);
    EXPECT_EQ(staff_spell.get_max_damage(), 40);
}

TEST_F(MagicWeaponTest, MagicWeaponManaCost) {
    EXPECT_EQ(staff.get_mana_cost(), 30);
    EXPECT_EQ(healing_wand.get_mana_cost(), 20);
}

TEST_F(MagicWeaponTest, HealingSpellMagicWeapon) {
    const Spell& wand_spell = healing_wand.get_spell();
    EXPECT_EQ(wand_spell.get_name(), "Heal");
    EXPECT_TRUE(wand_spell.does_heal());
    EXPECT_FALSE(wand_spell.does_damage());
}

TEST_F(MagicWeaponTest, DamageSpellMagicWeapon) {
    const Spell& staff_spell = staff.get_spell();
    EXPECT_TRUE(staff_spell.does_damage());
    EXPECT_FALSE(staff_spell.does_heal());
}


class DefensiveItemTest: public ::testing::Test {
 protected:
    DefensiveItem leather_armor{"Leather Armor", 5, 10, EquipmentSlot::ARMOR};
    DefensiveItem steel_armor{"Steel Armor", 15, 25, EquipmentSlot::ARMOR};
    DefensiveItem helmet{"Iron Helmet", 8, 15, EquipmentSlot::HELMET};
    DefensiveItem shield{"Wooden Shield", 3, 8, EquipmentSlot::SHIELD};
};

TEST_F(DefensiveItemTest, DefensiveItemArmorSlot) {
    auto slot = leather_armor.get_slot();
    ASSERT_TRUE(slot.has_value());
    EXPECT_EQ(slot.value(), EquipmentSlot::ARMOR);
}

TEST_F(DefensiveItemTest, DefensiveItemHelmetSlot) {
    auto slot = helmet.get_slot();
    ASSERT_TRUE(slot.has_value());
    EXPECT_EQ(slot.value(), EquipmentSlot::HELMET);
}

TEST_F(DefensiveItemTest, DefensiveItemShieldSlot) {
    auto slot = shield.get_slot();
    ASSERT_TRUE(slot.has_value());
    EXPECT_EQ(slot.value(), EquipmentSlot::SHIELD);
}


class ConsumableItemTest: public ::testing::Test {
 protected:
    ConsumableItem health_potion{"Health Potion", ConsumableType::HEALTH, 50};
    ConsumableItem mana_potion{"Mana Potion", ConsumableType::MANA, 40};
    ConsumableItem super_health_potion{"Super Health Potion", ConsumableType::HEALTH, 100};
};

TEST_F(ConsumableItemTest, ConsumableItemInitialization) {
    EXPECT_EQ(health_potion.get_name(), "Health Potion");
    EXPECT_EQ(mana_potion.get_name(), "Mana Potion");
}

TEST_F(ConsumableItemTest, ConsumableItemType) {
    EXPECT_EQ(health_potion.get_type(), ConsumableType::HEALTH);
    EXPECT_EQ(mana_potion.get_type(), ConsumableType::MANA);
}

TEST_F(ConsumableItemTest, ConsumableItemRestoreAmount) {
    EXPECT_EQ(health_potion.get_restore(), 50);
    EXPECT_EQ(mana_potion.get_restore(), 40);
    EXPECT_EQ(super_health_potion.get_restore(), 100);
}

TEST_F(ConsumableItemTest, ConsumableItemIsNotEquippable) {
    auto slot = health_potion.get_slot();
    EXPECT_FALSE(slot.has_value());
}

TEST_F(ConsumableItemTest, HealthPotionType) {
    EXPECT_EQ(health_potion.get_type(), ConsumableType::HEALTH);
}

TEST_F(ConsumableItemTest, ManaPotionType) {
    EXPECT_EQ(mana_potion.get_type(), ConsumableType::MANA);
}
