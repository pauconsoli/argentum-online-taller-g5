#include <memory>
#include <vector>

#include "common/npc_type.h"
#include "common/position.h"
#include "gtest/gtest.h"
#include "server/game/bank.h"
#include "server/game/items/item_registry.h"
#include "server/game/npcs/banker.h"
#include "server/game/npcs/city_npc.h"
#include "server/game/npcs/hostile_npc.h"
#include "server/game/npcs/merchant.h"
#include "server/game/npcs/priest.h"
#include "server/game/player.h"
#include "server/world/zone_type.h"

class NPCTest: public ::testing::Test {
 protected:
    Position pos{10, 10};
    std::vector<ZoneType> zones{ZoneType::ALL};

    HostileNPC goblin{1, "Goblin", NPCType::GOBLIN, 3, 40, 2, 10, 3, 8, 5, zones, pos};

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
    Priest priest{2, NPCType::PRIEST, pos};
    Merchant merchant{3, NPCType::MERCHANT, pos};
    Banker banker{4, NPCType::BANKER, pos};
    Bank bank;

    Player player{100, "P1", PlayerRace::HUMAN, PlayerClass::MAGE, 5, 100, 150, 10, 10,
                  15,  10,   Position{10, 11}};

    void SetUp() override {
        player.set_initial_stats(100, 150);
    }
};

TEST_F(CityNPCTest, CityNPCsAreFriendlyAndStill) {
    EXPECT_FALSE(priest.is_hostile());
    EXPECT_FALSE(merchant.is_hostile());
    EXPECT_FALSE(banker.is_hostile());

    auto p_behavior = priest.update(1.0f, {&player});
    EXPECT_EQ(p_behavior.action, NPCAction::STILL);

    auto m_behavior = merchant.update(1.0f, {&player});
    EXPECT_EQ(m_behavior.action, NPCAction::STILL);

    auto b_behavior = banker.update(1.0f, {&player});
    EXPECT_EQ(b_behavior.action, NPCAction::STILL);
}

TEST_F(CityNPCTest, PriestHealSuccess) {
    player.receive_damage(30);
    player.consume_mana(40);

    InteractResult res = priest.interact(NPCInteraction::HEAL, "", 0, player, bank);

    EXPECT_EQ(res.status, InteractStatus::SUCCESS);
    EXPECT_EQ(player.get_current_hp(), 100);
    EXPECT_EQ(player.get_current_mana(), 150);
}

TEST_F(CityNPCTest, PriestHealAlreadyFull) {
    InteractResult res = priest.interact(NPCInteraction::HEAL, "", 0, player, bank);
    EXPECT_EQ(res.status, InteractStatus::ALREADY_FULL);
}

TEST_F(CityNPCTest, PriestHealDead) {
    player.receive_damage(100);
    InteractResult res = priest.interact(NPCInteraction::HEAL, "", 0, player, bank);
    EXPECT_EQ(res.status, InteractStatus::PLAYER_DEAD);
}

TEST_F(CityNPCTest, PriestResurrectSuccess) {
    player.receive_damage(100);
    EXPECT_TRUE(player.is_dead());

    InteractResult res = priest.interact(NPCInteraction::RESURRECT, "", 0, player, bank);

    EXPECT_EQ(res.status, InteractStatus::SUCCESS);
    EXPECT_FALSE(player.is_dead());
    EXPECT_EQ(player.get_current_hp(), 100);
    EXPECT_EQ(player.get_current_mana(), 150);
}

TEST_F(CityNPCTest, PriestResurrectAlive) {
    InteractResult res = priest.interact(NPCInteraction::RESURRECT, "", 0, player, bank);
    EXPECT_EQ(res.status, InteractStatus::PLAYER_NOT_DEAD);
}

TEST_F(CityNPCTest, PriestBuyItemNotFound) {
    InteractResult res = priest.interact(NPCInteraction::BUY, "ItemInexistente", 1, player, bank);
    EXPECT_EQ(res.status, InteractStatus::ITEM_NOT_FOUND);
}

TEST_F(CityNPCTest, PriestListSuccess) {
    InteractResult res = priest.interact(NPCInteraction::LIST, "", 0, player, bank);
    EXPECT_EQ(res.status, InteractStatus::SUCCESS);
}

TEST_F(CityNPCTest, MerchantBuyItemNotFound) {
    InteractResult res = merchant.interact(NPCInteraction::BUY, "ItemInexistente", 1, player, bank);
    EXPECT_EQ(res.status, InteractStatus::ITEM_NOT_FOUND);
}

TEST_F(CityNPCTest, MerchantBuySellDeadPlayer) {
    player.receive_damage(100);

    InteractResult buy_res = merchant.interact(NPCInteraction::BUY, "Espada", 1, player, bank);
    EXPECT_EQ(buy_res.status, InteractStatus::PLAYER_DEAD);

    InteractResult sell_res = merchant.interact(NPCInteraction::SELL, "Espada", 1, player, bank);
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

    InteractResult res = merchant.interact(NPCInteraction::BUY, "PocionPrueba", 1, player, bank);

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

    InteractResult res = merchant.interact(NPCInteraction::BUY, "EspadaCara", 1, player, bank);

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
        merchant.interact(NPCInteraction::SELL, "PocionPruebaVenta", 1, player, bank);

    EXPECT_EQ(res.status, InteractStatus::SUCCESS);
    EXPECT_EQ(res.item_name, "PocionPruebaVenta");
    EXPECT_EQ(res.gold_amount, 30u);
    EXPECT_EQ(player.get_gold(), initial_gold + 30u);
    EXPECT_EQ(player.get_inventory().get_size(), 0);
}

TEST_F(CityNPCTest, PriestBuyPotionsSuccess) {
    player.add_gold(50);
    InteractResult res = priest.interact(NPCInteraction::BUY, "PocionPrueba", 1, player, bank);
    EXPECT_EQ(res.status, InteractStatus::SUCCESS);
    EXPECT_EQ(player.get_gold(), 0u);
}

TEST_F(CityNPCTest, BankerDepositGoldSuccess) {
    player.add_gold(100);
    InteractResult res = banker.interact(NPCInteraction::DEPOSIT_GOLD, "", 50, player, bank);

    EXPECT_EQ(res.status, InteractStatus::SUCCESS);
    EXPECT_EQ(player.get_gold(), 50u);
    EXPECT_EQ(bank.get_gold_from_safe(player.get_id()), 50u);
}

TEST_F(CityNPCTest, BankerDepositGoldInsufficient) {
    player.add_gold(10);
    InteractResult res = banker.interact(NPCInteraction::DEPOSIT_GOLD, "", 50, player, bank);

    EXPECT_EQ(res.status, InteractStatus::INSUFFICIENT_GOLD);
    EXPECT_EQ(player.get_gold(), 10u);
}

TEST_F(CityNPCTest, BankerDepositItemSuccess) {
    ItemRegistry::ItemTemplate tpl;
    tpl.name = "PocionBanco";
    tpl.type = "consumable";
    tpl.consumable_type = "health";
    ItemRegistry::get_instance().add_template(tpl.name, tpl);

    auto item = ItemRegistry::get_instance().create_item(tpl.name);
    player.get_inventory().add_item(std::move(item), 1);

    InteractResult res =
        banker.interact(NPCInteraction::DEPOSIT_ITEM, "PocionBanco", 0, player, bank);

    EXPECT_EQ(res.status, InteractStatus::SUCCESS);
    EXPECT_EQ(player.get_inventory().get_size(), 0);

    auto catalog = bank.list_items_from_safe(player.get_id());
    EXPECT_EQ(catalog.size(), 1);
    EXPECT_EQ(catalog[0], "PocionBanco");
}

TEST_F(CityNPCTest, BankerDepositItemNotFound) {
    InteractResult res = banker.interact(NPCInteraction::DEPOSIT_ITEM, "NoExiste", 0, player, bank);
    EXPECT_EQ(res.status, InteractStatus::ITEM_NOT_FOUND);
}

TEST_F(CityNPCTest, BankerWithdrawGoldSuccess) {
    bank.deposit_gold(player.get_id(), 100);
    EXPECT_EQ(player.get_gold(), 0u);

    InteractResult res = banker.interact(NPCInteraction::WITHDRAW_GOLD, "", 40, player, bank);

    EXPECT_EQ(res.status, InteractStatus::SUCCESS);
    EXPECT_EQ(player.get_gold(), 40u);
    EXPECT_EQ(bank.get_gold_from_safe(player.get_id()), 60u);
}

TEST_F(CityNPCTest, BankerWithdrawGoldInsufficient) {
    bank.deposit_gold(player.get_id(), 20);

    InteractResult res = banker.interact(NPCInteraction::WITHDRAW_GOLD, "", 50, player, bank);

    EXPECT_EQ(res.status, InteractStatus::INSUFFICIENT_GOLD);
    EXPECT_EQ(bank.get_gold_from_safe(player.get_id()), 20u);
}

TEST_F(CityNPCTest, BankerWithdrawItemSuccess) {
    ItemRegistry::ItemTemplate tpl;
    tpl.name = "ArmaBanco";
    tpl.type = "weapon";
    ItemRegistry::get_instance().add_template(tpl.name, tpl);

    auto item = ItemRegistry::get_instance().create_item(tpl.name);
    bank.deposit_item(player.get_id(), std::move(item));

    InteractResult res =
        banker.interact(NPCInteraction::WITHDRAW_ITEM, "ArmaBanco", 0, player, bank);

    EXPECT_EQ(res.status, InteractStatus::SUCCESS);
    EXPECT_EQ(player.get_inventory().get_size(), 1);
    EXPECT_TRUE(bank.list_items_from_safe(player.get_id()).empty());
}

TEST_F(CityNPCTest, BankerWithdrawItemNotFound) {
    InteractResult res =
        banker.interact(NPCInteraction::WITHDRAW_ITEM, "NoExiste", 0, player, bank);
    EXPECT_EQ(res.status, InteractStatus::ITEM_NOT_FOUND);
}

TEST_F(CityNPCTest, BankerListSuccess) {
    bank.deposit_gold(player.get_id(), 500);

    ItemRegistry::ItemTemplate tpl;
    tpl.name = "CascoBanco";
    tpl.type = "defensive";
    tpl.slot = "helmet";
    ItemRegistry::get_instance().add_template(tpl.name, tpl);
    bank.deposit_item(player.get_id(), ItemRegistry::get_instance().create_item(tpl.name));

    InteractResult res = banker.interact(NPCInteraction::LIST, "", 0, player, bank);

    EXPECT_EQ(res.status, InteractStatus::SUCCESS);
    EXPECT_EQ(res.gold_amount, 500u);
    EXPECT_EQ(res.catalog.size(), 1);
    EXPECT_EQ(res.catalog[0], "CascoBanco");
}

TEST_F(CityNPCTest, BankerDeadPlayer) {
    player.receive_damage(100);

    InteractResult dep_g = banker.interact(NPCInteraction::DEPOSIT_GOLD, "", 10, player, bank);
    EXPECT_EQ(dep_g.status, InteractStatus::PLAYER_DEAD);

    InteractResult with_g = banker.interact(NPCInteraction::WITHDRAW_GOLD, "", 10, player, bank);
    EXPECT_EQ(with_g.status, InteractStatus::PLAYER_DEAD);

    InteractResult dep_i = banker.interact(NPCInteraction::DEPOSIT_ITEM, "A", 0, player, bank);
    EXPECT_EQ(dep_i.status, InteractStatus::PLAYER_DEAD);

    InteractResult with_i = banker.interact(NPCInteraction::WITHDRAW_ITEM, "A", 0, player, bank);
    EXPECT_EQ(with_i.status, InteractStatus::PLAYER_DEAD);

    InteractResult list = banker.interact(NPCInteraction::LIST, "", 0, player, bank);
    EXPECT_EQ(list.status, InteractStatus::PLAYER_DEAD);
}
