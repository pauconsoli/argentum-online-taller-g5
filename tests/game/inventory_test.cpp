#include "server/game/inventory.h"

#include "common/position.h"
#include "gtest/gtest.h"
#include "server/game/items/consumable_item.h"
#include "server/game/items/defensive_item.h"
#include "server/game/items/magic_weapon.h"
#include "server/game/items/normal_weapon.h"
#include "server/game/items/spell.h"
#include "server/game/player.h"

class InventoryTest: public ::testing::Test {
 protected:
    Position position{5, 10};

    Player player{1,  "Thoros of Myr", PlayerRace::HUMAN, PlayerClass::MAGE, 5, 100, 80, 10, 8, 16,
                  15, position};

    Inventory& inventory = player.get_inventory();

    NormalWeapon sword{"Espada", 5, 15, false};
    NormalWeapon axe{"Hacha", 8, 20, false};

    Spell fireball{"Fireball", 20, 40, 0, 0};
    MagicWeapon staff{"Báculo", std::make_unique<Spell>(fireball), 30};

    DefensiveItem armor{"Armadura", 5, 10, EquipmentSlot::ARMOR};
    DefensiveItem helmet{"Casco", 3, 6, EquipmentSlot::HELMET};

    ConsumableItem potion{"Poción de Vida", ConsumableType::HEALTH, 30};
};

TEST_F(InventoryTest, AddItemToEmptyInventory) {
    auto item = std::make_unique<NormalWeapon>("Espada", 5, 15, false);
    bool result = inventory.add_item(std::move(item));
    EXPECT_TRUE(result);
    EXPECT_EQ(inventory.get_size(), 1);
}

TEST_F(InventoryTest, AddMultipleItems) {
    auto item1 = std::make_unique<NormalWeapon>("Espada", 5, 15, false);
    auto item2 = std::make_unique<NormalWeapon>("Hacha", 8, 20, false);
    auto item3 = std::make_unique<NormalWeapon>("Lanza", 6, 18, false);

    EXPECT_TRUE(inventory.add_item(std::move(item1)));
    EXPECT_TRUE(inventory.add_item(std::move(item2)));
    EXPECT_TRUE(inventory.add_item(std::move(item3)));

    EXPECT_EQ(inventory.get_size(), 3);
}

TEST_F(InventoryTest, AddItemWhenFull) {
    for (int i = 0; i < MAX_INVENTORY_ITEMS; i++) {
        auto item = std::make_unique<NormalWeapon>("Espada " + std::to_string(i), 5, 15, false);
        inventory.add_item(std::move(item));
    }
    EXPECT_TRUE(inventory.is_full());

    auto extra = std::make_unique<NormalWeapon>("Extra", 5, 15, false);
    bool result = inventory.add_item(std::move(extra));
    EXPECT_FALSE(result);
    EXPECT_EQ(inventory.get_size(), MAX_INVENTORY_ITEMS);
}

TEST_F(InventoryTest, EquipItemFromInventory) {
    auto item = std::make_unique<NormalWeapon>("Espada", 5, 15, false);
    Item* ptr = item.get();
    inventory.add_item(std::move(item));

    EXPECT_EQ(inventory.get_size(), 1);
    EXPECT_TRUE(inventory.equip(*ptr, player));
    EXPECT_EQ(inventory.get_size(),
              1);  // el item se queda en el inventario, solo marcado como equipado
    EXPECT_TRUE(inventory.slot_has_item(EquipmentSlot::WEAPON));
}

TEST_F(InventoryTest, EquipItemNotInInventory) {
    NormalWeapon item{"Espada", 5, 15, false};
    bool result = inventory.equip(item, player);
    EXPECT_FALSE(result);  // no está en el inventario
}

TEST_F(InventoryTest, EquipConsumable) {
    auto potion = std::make_unique<ConsumableItem>("Poción", ConsumableType::HEALTH, 30);
    Item* ptr = potion.get();
    inventory.add_item(std::move(potion));

    EXPECT_EQ(inventory.get_size(), 1);
    bool result = inventory.equip(*ptr, player);

    EXPECT_TRUE(result);                 // el consumible se usó
    EXPECT_EQ(inventory.get_size(), 0);  // se consumió, no está en inventario
}

TEST_F(InventoryTest, SwapEquippedItem) {
    auto sword_item = std::make_unique<NormalWeapon>("Espada", 5, 15, false);
    Item* sword_ptr = sword_item.get();
    inventory.add_item(std::move(sword_item));
    inventory.equip(*sword_ptr, player);

    EXPECT_EQ(inventory.get_size(), 1);  // item permanece en inventario

    auto axe_item = std::make_unique<NormalWeapon>("Hacha", 8, 20, false);
    Item* axe_ptr = axe_item.get();
    inventory.add_item(std::move(axe_item));

    bool result = inventory.equip(*axe_ptr, player);
    EXPECT_TRUE(result);
    EXPECT_EQ(inventory.get_size(), 2);  // ambos items siguen en el inventario
    EXPECT_EQ(inventory.get_equipped_item(EquipmentSlot::WEAPON), axe_ptr);
}

TEST_F(InventoryTest, SwapWhenInventoryFull) {
    for (int i = 0; i < MAX_INVENTORY_ITEMS - 1; i++) {
        auto item = std::make_unique<NormalWeapon>("Espada " + std::to_string(i), 5, 15, false);
        inventory.add_item(std::move(item));
    }

    auto sword = std::make_unique<NormalWeapon>("Mi Espada", 5, 15, false);
    Item* sword_ptr = sword.get();
    inventory.add_item(std::move(sword));
    EXPECT_EQ(inventory.get_size(), MAX_INVENTORY_ITEMS);

    inventory.equip(*sword_ptr, player);  // se marca como equipado, sigue en inventario
    EXPECT_EQ(inventory.get_size(), MAX_INVENTORY_ITEMS);

    // El inventario está lleno, no se puede agregar más items
    auto axe = std::make_unique<NormalWeapon>("Hacha", 8, 20, false);
    bool result = inventory.add_item(std::move(axe));
    EXPECT_FALSE(result);  // no hay espacio
    EXPECT_EQ(inventory.get_size(), MAX_INVENTORY_ITEMS);
}

TEST_F(InventoryTest, UnequipToEmptyInventory) {
    auto sword = std::make_unique<NormalWeapon>("Espada", 5, 15, false);
    Item* ptr = sword.get();
    inventory.add_item(std::move(sword));
    inventory.equip(*ptr, player);

    EXPECT_EQ(inventory.get_size(), 1);  // item sigue en el inventario
    bool result = inventory.unequip(EquipmentSlot::WEAPON);
    EXPECT_TRUE(result);
    EXPECT_EQ(inventory.get_size(), 1);  // item sigue en el inventario, solo se desmarca
    EXPECT_FALSE(inventory.slot_has_item(EquipmentSlot::WEAPON));
}

TEST_F(InventoryTest, UnequipFromEmptySlot) {
    bool result = inventory.unequip(EquipmentSlot::WEAPON);
    EXPECT_FALSE(result);  // no hay nada equipado
}

TEST_F(InventoryTest, UnequipWhenInventoryFull) {
    for (int i = 0; i < MAX_INVENTORY_ITEMS; i++) {
        auto item = std::make_unique<NormalWeapon>("Arma " + std::to_string(i), 5, 15, false);
        inventory.add_item(std::move(item));
    }
    EXPECT_TRUE(inventory.is_full());

    // Equipar el primer item (ya está en el inventario)
    Item* first_item = inventory.get_slots()[0].item.get();
    inventory.equip(*first_item, player);
    EXPECT_TRUE(inventory.slot_has_item(EquipmentSlot::WEAPON));

    // Desquipar simplemente desmarca el item (sigue en inventario)
    bool result = inventory.unequip(EquipmentSlot::WEAPON);
    EXPECT_TRUE(result);
    EXPECT_EQ(inventory.get_size(), MAX_INVENTORY_ITEMS);  // el inventario sigue lleno
    EXPECT_FALSE(inventory.slot_has_item(EquipmentSlot::WEAPON));
}

// ver después que hacer si se droppea
