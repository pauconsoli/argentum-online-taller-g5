#include <memory>
#include <string>

#include "gtest/gtest.h"
#include "server/game/game_config.h"
#include "server/game/game_formulas.h"
#include "server/game/items/magic_weapon.h"
#include "server/game/items/spell.h"
#include "server/game/items/weapon.h"
#include "server/game/player.h"
#include "server/world/world.h"

class AttackTest: public ::testing::Test {
 protected:
    World world{100, 100};

    // Método auxiliar para no repetir código de creación
    Player* test_add_player(uint32_t id, int x, int y, int level = 20,
                            PlayerClass klass = PlayerClass::WARRIOR) {
        auto player =
            std::make_unique<Player>(id, "Player" + std::to_string(id), PlayerRace::HUMAN, klass,
                                     level, 1000, 1000, 20, 20, 20, 20, Position{x, y});
        world.add_player(std::move(player));
        return world.get_player(id);
    }
};

TEST_F(AttackTest, CloseAttackSuccess) {
    Player* attacker = test_add_player(1, 10, 10);
    Player* target = test_add_player(2, 10, 11);

    int initial_hp = target->get_current_hp();
    uint64_t initial_exp = attacker->get_experience();

    AttackResult result = world.attack(1, 2);

    if (!result.evaded) {
        EXPECT_GT(result.damage, 0);
        EXPECT_EQ(target->get_current_hp(), initial_hp - result.damage);
        EXPECT_GT(attacker->get_experience(), initial_exp);  // ganó XP por atacar
    } else {
        EXPECT_EQ(result.damage, 0);
        EXPECT_EQ(target->get_current_hp(), initial_hp);
    }
}

TEST_F(AttackTest, CloseAttackOutOfRange) {
    test_add_player(1, 10, 10);
    test_add_player(2, 10, 15);

    // distancia > 1 sin arma a distancia, debe lanzar error
    AttackResult res = world.attack(1, 2);
    EXPECT_EQ(res.status, AttackStatus::OUT_OF_RANGE);
}

TEST_F(AttackTest, RangedAttackInRangeWithMagicWeapon) {
    Player* attacker = test_add_player(1, 10, 10, 20, PlayerClass::MAGE);
    test_add_player(2, 10, 15);

    auto spell = std::make_unique<Spell>("Fireball", 20, 40, 0, 0);
    auto staff = std::make_unique<MagicWeapon>("Staff", std::move(spell), 10);
    Item* staff_ptr = staff.get();

    attacker->get_inventory().add_item(std::move(staff));
    attacker->equip(*staff_ptr);

    // arma mágica tiene rango, no debería fallar por distancia
    AttackResult res = world.attack(1, 2);
    EXPECT_EQ(res.status, AttackStatus::SUCCESS);
}

TEST_F(AttackTest, NewbieCannotAttackOrBeAttacked) {
    test_add_player(1, 10, 10, 5);  // newbie
    test_add_player(2, 10, 11, 20);

    AttackResult res1 = world.attack(1, 2);  // newbie atacando
    EXPECT_EQ(res1.status, AttackStatus::NEWBIE_PROTECTION);

    AttackResult res2 = world.attack(2, 1);  // atacando al newbie
    EXPECT_EQ(res2.status, AttackStatus::NEWBIE_PROTECTION);
}

TEST_F(AttackTest, MaxLevelDifference) {
    test_add_player(1, 10, 10, 20);
    test_add_player(2, 10, 11, 40);  // diferencia de 20 niveles (> 10)

    AttackResult res = world.attack(1, 2);
    EXPECT_EQ(res.status, AttackStatus::LEVEL_DIFFERENCE);
}

TEST_F(AttackTest, DeadTargetCannotBeAttacked) {
    test_add_player(1, 10, 10);
    Player* target = test_add_player(2, 10, 11);

    target->receive_damage(9999);
    EXPECT_TRUE(target->is_dead());

    AttackResult res = world.attack(1, 2);
    EXPECT_EQ(res.status, AttackStatus::DEAD);
}

TEST_F(AttackTest, DeadAttackerCannotAttack) {
    Player* attacker = test_add_player(1, 10, 10);
    test_add_player(2, 10, 11);

    attacker->receive_damage(9999);
    EXPECT_TRUE(attacker->is_dead());

    AttackResult res = world.attack(1, 2);
    EXPECT_EQ(res.status, AttackStatus::DEAD);
}

TEST_F(AttackTest, MagicWeaponConsumesMana) {
    Player* attacker = test_add_player(1, 10, 10, 20, PlayerClass::MAGE);
    test_add_player(2, 10, 11);

    auto spell = std::make_unique<Spell>("Fireball", 20, 40, 0, 0);
    auto staff = std::make_unique<MagicWeapon>("Staff", std::move(spell), 15);
    Item* staff_ptr = staff.get();

    attacker->get_inventory().add_item(std::move(staff));
    attacker->equip(*staff_ptr);

    int initial_mana = attacker->get_current_mana();
    world.attack(1, 2);

    EXPECT_EQ(attacker->get_current_mana(), initial_mana - 15);
}

TEST_F(AttackTest, MagicWeaponInsufficientManaFails) {
    Player* attacker = test_add_player(1, 10, 10, 20, PlayerClass::MAGE);
    test_add_player(2, 10, 11);

    auto spell = std::make_unique<Spell>("Fireball", 20, 40, 0, 0);
    auto staff = std::make_unique<MagicWeapon>("Staff", std::move(spell), 5000);  // Costo imposible
    Item* staff_ptr = staff.get();

    attacker->get_inventory().add_item(std::move(staff));
    attacker->equip(*staff_ptr);

    AttackResult res = world.attack(1, 2);
    EXPECT_EQ(res.status, AttackStatus::NO_MANA);
}

TEST_F(AttackTest, DeathFlowPenalties) {
    test_add_player(1, 10, 10);

    Player* target = test_add_player(2, 10, 11, 19);
    target->receive_damage(target->get_current_hp() - 1);

    // experiencia de Nivel 20 (límite del nivel 19 +algo)
    target->add_experience(GameFormulas::calculate_level_up_limit(19) + 5000);
    EXPECT_EQ(target->get_level(), 20);
    uint64_t initial_exp = target->get_experience();

    uint64_t max_safe = GameFormulas::calculate_max_gold(*target);
    target->add_gold(max_safe + 5000);  // 5000 oro en exceso

    auto sword = std::make_unique<Weapon>("DeathSword", 10, 20, false);
    target->get_inventory().add_item(std::move(sword));
    EXPECT_EQ(target->get_inventory().get_size(), 1);

    bool died = false;
    for (int i = 0; i < 50; ++i) {
        if (target->is_dead())
            break;
        AttackResult res = world.attack(1, 2);
        if (res.target_died) {
            died = true;
            break;
        }
    }

    ASSERT_TRUE(died);
    EXPECT_TRUE(target->is_dead());

    // validaciones de muerte
    EXPECT_EQ(target->get_gold(), max_safe);           // no tiene exceso de oro
    EXPECT_EQ(target->get_inventory().get_size(), 0);  // inventario vacío
    EXPECT_LT(target->get_experience(), initial_exp);  // pierde experiencia
}

TEST_F(AttackTest, AttackerGetsRewardsOnKill) {
    Player* attacker = test_add_player(1, 10, 10);
    Player* target = test_add_player(2, 10, 11);

    target->receive_damage(target->get_current_hp() - 1);

    uint64_t initial_exp = attacker->get_experience();

    bool died = false;
    for (int i = 0; i < 50; ++i) {
        if (target->is_dead())
            break;
        AttackResult res = world.attack(1, 2);
        if (res.target_died) {
            died = true;
            break;
        }
    }

    ASSERT_TRUE(died);
    EXPECT_GT(attacker->get_experience(), initial_exp);
}

TEST_F(AttackTest, SelfAttackNormalFails) {
    test_add_player(1, 10, 10);

    AttackResult res = world.attack(1, 1);
    EXPECT_EQ(res.status, AttackStatus::INVALID_TARGET);
}

TEST_F(AttackTest, HealingSpellSuccess) {
    Player* attacker = test_add_player(1, 10, 10, 20, PlayerClass::MAGE);
    Player* target = test_add_player(2, 10, 11);

    target->receive_damage(50);
    int initial_hp = target->get_current_hp();
    int initial_mana = attacker->get_current_mana();

    auto spell = std::make_unique<Spell>("Heal", 0, 0, 20, 40);
    auto flute = std::make_unique<MagicWeapon>("Flute", std::move(spell), 10);
    Item* flute_ptr = flute.get();
    attacker->get_inventory().add_item(std::move(flute));
    attacker->equip(*flute_ptr);

    AttackResult result = world.heal(1, 2);

    EXPECT_TRUE(result.is_healing);
    EXPECT_GT(result.heal_amount, 0);
    EXPECT_EQ(target->get_current_hp(), initial_hp + result.heal_amount);
    EXPECT_EQ(attacker->get_current_mana(), initial_mana - 10);
}
