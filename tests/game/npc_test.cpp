#include <memory>
#include <vector>

#include "common/position.h"
#include "gtest/gtest.h"
#include "server/game/items/item_registry.h"
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


// TESTS CITY NPCS (Priest, Merchant, Banker)
// Queda bank dummy porque tdavia no lo implementé

struct DummyBank {
    char pad[1024];
};

class CityNPCTest: public ::testing::Test {
 protected:
    Position pos{10, 10};
    Priest priest{2, pos};
    Merchant merchant{3, pos};

    Player player{100, "P1", PlayerRace::HUMAN, PlayerClass::MAGE, 5, 100, 150, 10, 10,
                  15,  10,   Position{10, 11}};

    DummyBank dummy_bank_storage;
    Bank& dummy_bank = reinterpret_cast<Bank&>(dummy_bank_storage);

    void SetUp() override {
        player.set_initial_stats(100, 150);
    }
};

TEST_F(CityNPCTest, CityNPCsAreFriendlyAndStill) {
    EXPECT_FALSE(priest.is_hostile());
    EXPECT_FALSE(merchant.is_hostile());

    auto p_behavior = priest.update(1.0f, {&player});
    EXPECT_EQ(p_behavior.action, NPCAction::STILL);

    auto m_behavior = merchant.update(1.0f, {&player});
    EXPECT_EQ(m_behavior.action, NPCAction::STILL);
}

TEST_F(CityNPCTest, PriestHealSuccess) {
    player.receive_damage(30);
    player.consume_mana(40);

    InteractResult res = priest.interact(NPCInteraction::HEAL, "", 0, player, dummy_bank);

    EXPECT_EQ(res.status, InteractStatus::SUCCESS);
    EXPECT_EQ(player.get_current_hp(), 100);
    EXPECT_EQ(player.get_current_mana(), 150);
}

TEST_F(CityNPCTest, PriestHealAlreadyFull) {
    InteractResult res = priest.interact(NPCInteraction::HEAL, "", 0, player, dummy_bank);
    EXPECT_EQ(res.status, InteractStatus::ALREADY_FULL);
}

TEST_F(CityNPCTest, PriestHealDead) {
    player.receive_damage(100);
    InteractResult res = priest.interact(NPCInteraction::HEAL, "", 0, player, dummy_bank);
    EXPECT_EQ(res.status, InteractStatus::PLAYER_DEAD);
}

TEST_F(CityNPCTest, PriestResurrectSuccess) {
    player.receive_damage(100);
    EXPECT_TRUE(player.is_dead());

    InteractResult res = priest.interact(NPCInteraction::RESURRECT, "", 0, player, dummy_bank);

    EXPECT_EQ(res.status, InteractStatus::SUCCESS);
    EXPECT_FALSE(player.is_dead());
    EXPECT_EQ(player.get_current_hp(), 100);
    EXPECT_EQ(player.get_current_mana(), 150);
}

TEST_F(CityNPCTest, PriestResurrectAlive) {
    InteractResult res = priest.interact(NPCInteraction::RESURRECT, "", 0, player, dummy_bank);
    EXPECT_EQ(res.status, InteractStatus::PLAYER_NOT_DEAD);
}

TEST_F(CityNPCTest, PriestBuyItemNotFound) {
    InteractResult res =
        priest.interact(NPCInteraction::BUY, "ItemInexistente", 1, player, dummy_bank);
    EXPECT_EQ(res.status, InteractStatus::ITEM_NOT_FOUND);
}

TEST_F(CityNPCTest, PriestListSuccess) {
    InteractResult res = priest.interact(NPCInteraction::LIST, "", 0, player, dummy_bank);
    EXPECT_EQ(res.status, InteractStatus::SUCCESS);
}

TEST_F(CityNPCTest, MerchantBuyItemNotFound) {
    InteractResult res =
        merchant.interact(NPCInteraction::BUY, "ItemInexistente", 1, player, dummy_bank);
    EXPECT_EQ(res.status, InteractStatus::ITEM_NOT_FOUND);
}

TEST_F(CityNPCTest, MerchantBuySellDeadPlayer) {
    player.receive_damage(100);

    InteractResult buy_res =
        merchant.interact(NPCInteraction::BUY, "Espada", 1, player, dummy_bank);
    EXPECT_EQ(buy_res.status, InteractStatus::PLAYER_DEAD);

    InteractResult sell_res =
        merchant.interact(NPCInteraction::SELL, "Espada", 1, player, dummy_bank);
    EXPECT_EQ(sell_res.status, InteractStatus::PLAYER_DEAD);
}

TEST_F(CityNPCTest, MerchantBuySuccess) {
    // pongo un item
    ItemRegistry::ItemTemplate tpl;
    tpl.name = "PocionPrueba";
    tpl.type = "consumable";
    tpl.consumable_type = "health";
    tpl.price = 50;
    ItemRegistry::get_instance().add_template(tpl.name, tpl);

    player.add_gold(100);

    InteractResult res =
        merchant.interact(NPCInteraction::BUY, "PocionPrueba", 1, player, dummy_bank);

    EXPECT_EQ(res.status, InteractStatus::SUCCESS);
    EXPECT_EQ(res.item_name, "PocionPrueba");
    EXPECT_EQ(res.gold_amount, 50u);
    EXPECT_EQ(player.get_gold(), 50u);
    EXPECT_EQ(player.get_inventory().get_size(), 1);
}

TEST_F(CityNPCTest, MerchantBuyInsufficientGold) {
    ItemRegistry::ItemTemplate tpl;
    tpl.name = "EspadaCara";
    tpl.type = "weapon";
    tpl.price = 9999;
    ItemRegistry::get_instance().add_template(tpl.name, tpl);

    EXPECT_EQ(player.get_gold(), 0u);

    InteractResult res =
        merchant.interact(NPCInteraction::BUY, "EspadaCara", 1, player, dummy_bank);

    EXPECT_EQ(res.status, InteractStatus::INSUFFICIENT_GOLD);
    EXPECT_EQ(player.get_inventory().get_size(), 0);
}

TEST_F(CityNPCTest, MerchantSellSuccess) {
    ItemRegistry::ItemTemplate tpl;
    tpl.name = "PocionPruebaVenta";
    tpl.type = "consumable";
    tpl.consumable_type = "health";
    tpl.price = 60;
    ItemRegistry::get_instance().add_template(tpl.name, tpl);

    auto item = ItemRegistry::get_instance().create_item(tpl.name);
    player.get_inventory().add_item(std::move(item), 1);  // le doy el item para vender

    uint64_t initial_gold = player.get_gold();

    InteractResult res =
        merchant.interact(NPCInteraction::SELL, "PocionPruebaVenta", 1, player, dummy_bank);

    EXPECT_EQ(res.status, InteractStatus::SUCCESS);
    EXPECT_EQ(res.item_name, "PocionPruebaVenta");
    EXPECT_EQ(res.gold_amount, 30u);
    EXPECT_EQ(player.get_gold(), initial_gold + 30u);
    EXPECT_EQ(player.get_inventory().get_size(), 0);
}

TEST_F(CityNPCTest, PriestBuyPotionsSuccess) {
    player.add_gold(50);
    InteractResult res =
        priest.interact(NPCInteraction::BUY, "PocionPrueba", 1, player, dummy_bank);
    EXPECT_EQ(res.status, InteractStatus::SUCCESS);
    EXPECT_EQ(player.get_gold(), 0u);
}
