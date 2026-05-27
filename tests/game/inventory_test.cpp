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


TEST_F(InventoryTest, AddStackableItem) {
    auto potion1 = std::make_unique<ConsumableItem>("Poción de Vida", ConsumableType::HEALTH, 30);
    EXPECT_TRUE(inventory.add_item(std::move(potion1)));
    EXPECT_EQ(inventory.get_size(), 1);
    EXPECT_EQ(inventory.get_slots()[0].quantity, 1);

    auto potion2 = std::make_unique<ConsumableItem>("Poción de Vida", ConsumableType::HEALTH, 30);
    EXPECT_TRUE(inventory.add_item(std::move(potion2)));
    EXPECT_EQ(inventory.get_size(), 1);
    EXPECT_EQ(inventory.get_slots()[0].quantity, 2);  // pero con cantidad 2
}

TEST_F(InventoryTest, AddMultipleStackableItems) {
    auto potion = std::make_unique<ConsumableItem>("Poción de Vida", ConsumableType::HEALTH, 30);
    EXPECT_TRUE(inventory.add_item(std::move(potion), 5));
    EXPECT_EQ(inventory.get_size(), 1);
    EXPECT_EQ(inventory.get_slots()[0].quantity, 5);

    auto potion2 = std::make_unique<ConsumableItem>("Poción de Vida", ConsumableType::HEALTH, 30);
    EXPECT_TRUE(inventory.add_item(std::move(potion2), 3));
    EXPECT_EQ(inventory.get_size(), 1);
    EXPECT_EQ(inventory.get_slots()[0].quantity, 8);
}

TEST_F(InventoryTest, StackableAndNonStackableItems) {
    auto potion = std::make_unique<ConsumableItem>("Poción de Vida", ConsumableType::HEALTH, 30);
    inventory.add_item(std::move(potion), 5);

    auto sword = std::make_unique<NormalWeapon>("Espada", 5, 15, false);
    inventory.add_item(std::move(sword));

    EXPECT_EQ(inventory.get_size(), 2);
    EXPECT_EQ(inventory.get_slots()[0].quantity, 5);
    EXPECT_EQ(inventory.get_slots()[1].quantity, 1);
}


TEST_F(InventoryTest, PopSlotReturnsInventorySlotWithQuantity) {
    auto potion = std::make_unique<ConsumableItem>("Poción de Vida", ConsumableType::HEALTH, 30);
    inventory.add_item(std::move(potion), 21);
    EXPECT_EQ(inventory.get_size(), 1);

    auto slot = inventory.pop_slot(0);
    EXPECT_NE(slot.item, nullptr);
    EXPECT_EQ(slot.quantity, 21);
    EXPECT_EQ(inventory.get_size(), 0);
}

TEST_F(InventoryTest, PopSlotInvalidIndex) {
    auto slot_invalid = inventory.pop_slot(999);
    EXPECT_EQ(slot_invalid.item, nullptr);
    EXPECT_EQ(slot_invalid.quantity, 0);
    EXPECT_EQ(inventory.get_size(), 0);
}


TEST_F(InventoryTest, UseConsumableWithMultipleInStack) {
    auto potion = std::make_unique<ConsumableItem>("Poción de Vida", ConsumableType::HEALTH, 30);
    Item* ptr = potion.get();
    inventory.add_item(std::move(potion), 5);
    EXPECT_EQ(inventory.get_slots()[0].quantity, 5);

    bool result = inventory.equip(*ptr, player);
    EXPECT_TRUE(result);
    EXPECT_EQ(inventory.get_size(), 1);
    EXPECT_EQ(inventory.get_slots()[0].quantity, 4);
}

TEST_F(InventoryTest, UseLastConsumableInStack) {
    auto potion = std::make_unique<ConsumableItem>("Poción de Vida", ConsumableType::HEALTH, 30);
    Item* ptr = potion.get();
    inventory.add_item(std::move(potion), 1);
    EXPECT_EQ(inventory.get_size(), 1);

    bool result = inventory.equip(*ptr, player);
    EXPECT_TRUE(result);
    EXPECT_EQ(inventory.get_size(), 0);
}


TEST_F(InventoryTest, RemoveItemFromStack) {
    auto potion = std::make_unique<ConsumableItem>("Poción de Vida", ConsumableType::HEALTH, 30);
    Item* ptr = potion.get();
    inventory.add_item(std::move(potion), 5);

    auto removed = inventory.remove_item(*ptr);
    EXPECT_EQ(removed, nullptr);  // quedan más --> retorna nullptr. ver si esto queda así
    EXPECT_EQ(inventory.get_size(), 1);
    EXPECT_EQ(inventory.get_slots()[0].quantity, 4);
}

TEST_F(InventoryTest, RemoveLastItemFromStack) {
    auto potion = std::make_unique<ConsumableItem>("Poción de Vida", ConsumableType::HEALTH, 30);
    Item* ptr = potion.get();
    inventory.add_item(std::move(potion), 1);

    auto removed = inventory.remove_item(*ptr);
    EXPECT_NE(removed, nullptr);  // es el último --> retorna el item. idem arriba
    EXPECT_EQ(inventory.get_size(), 0);
}

TEST_F(InventoryTest, RemoveItemNotInInventory) {
    NormalWeapon sword{"Espada", 5, 15, false};
    auto removed = inventory.remove_item(sword);
    EXPECT_EQ(removed, nullptr);
}

TEST_F(InventoryTest, CannotEquipNormalAndMagicWeaponTogether) {
    auto sword = std::make_unique<NormalWeapon>("Espada", 5, 15, false);
    Item* sword_ptr = sword.get();
    inventory.add_item(std::move(sword));

    EXPECT_TRUE(inventory.equip(*sword_ptr, player));
    EXPECT_EQ(inventory.get_equipped_item(EquipmentSlot::WEAPON), sword_ptr);

    auto spell = std::make_unique<Spell>("Fireball", 20, 40, 0, 0);
    auto staff = std::make_unique<MagicWeapon>("Báculo", std::move(spell), 30);
    Item* staff_ptr = staff.get();
    inventory.add_item(std::move(staff));

    EXPECT_TRUE(inventory.equip(*staff_ptr, player));
    EXPECT_EQ(inventory.get_equipped_item(EquipmentSlot::WEAPON), staff_ptr);

    EXPECT_TRUE(inventory.slot_has_item(EquipmentSlot::WEAPON));
    EXPECT_FALSE(inventory.get_equipped_item(EquipmentSlot::WEAPON) ==
                 sword_ptr);  // la espada no está equipada y el báculo si

    EXPECT_EQ(inventory.get_size(), 2);
}

TEST_F(InventoryTest, CannotEquipMultipleArmorInSameSlot) {
    auto armor1 = std::make_unique<DefensiveItem>("Armadura Ligera", 5, 10, EquipmentSlot::ARMOR);
    Item* armor1_ptr = armor1.get();
    inventory.add_item(std::move(armor1));

    EXPECT_TRUE(inventory.equip(*armor1_ptr, player));
    EXPECT_EQ(inventory.get_equipped_item(EquipmentSlot::ARMOR), armor1_ptr);

    auto armor2 = std::make_unique<DefensiveItem>("Armadura Pesada", 8, 16, EquipmentSlot::ARMOR);
    Item* armor2_ptr = armor2.get();
    inventory.add_item(std::move(armor2));

    EXPECT_TRUE(inventory.equip(*armor2_ptr, player));
    EXPECT_EQ(inventory.get_equipped_item(EquipmentSlot::ARMOR), armor2_ptr);
    EXPECT_FALSE(inventory.get_equipped_item(EquipmentSlot::ARMOR) == armor1_ptr);
    EXPECT_NE(inventory.get_slots()[0].equipped_slot,
              EquipmentSlot::ARMOR);  // el primer item ya no está equipado

    EXPECT_EQ(inventory.get_size(), 2);
}
