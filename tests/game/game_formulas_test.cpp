#include "server/game/game_formulas.h"

#include "common/position.h"
#include "gtest/gtest.h"
#include "server/game/player.h"

class GameFormulasTest: public ::testing::Test {
 protected:
    Position position{0, 0};

    Player human_mage{1,  "Merlin", PlayerRace::HUMAN, PlayerClass::MAGE, 5, 100, 80, 10, 8,
                      16, 15,       position};
    Player human_warrior{2,  "Leonidas", PlayerRace::HUMAN, PlayerClass::WARRIOR, 5, 100, 80, 10, 8,
                         16, 15,         position};
    Player elf_cleric{3,  "Baelor", PlayerRace::ELF, PlayerClass::CLERIC, 5, 100, 80, 10, 8,
                      16, 15,       position};
    Player dwarf_paladin{4,  "Gimli", PlayerRace::DWARF, PlayerClass::PALADIN, 5, 100, 80, 10, 8,
                         16, 15,      position};
};

TEST_F(GameFormulasTest, CalculateMaxHPHumanMage) {
    int max_hp = GameFormulas::calculate_max_hp(human_mage);
    EXPECT_EQ(max_hp, 52);
}

TEST_F(GameFormulasTest, CalculateMaxHPHumanWarrior) {
    int max_hp = GameFormulas::calculate_max_hp(human_warrior);
    EXPECT_EQ(max_hp, 112);
}

TEST_F(GameFormulasTest, CalculateMaxHPElfCleric) {
    int max_hp = GameFormulas::calculate_max_hp(elf_cleric);
    EXPECT_EQ(max_hp, 60);
}

TEST_F(GameFormulasTest, CalculateMaxHPDwarfPaladin) {
    int max_hp = GameFormulas::calculate_max_hp(dwarf_paladin);
    EXPECT_EQ(max_hp, 126);
}

TEST_F(GameFormulasTest, CalculateMaxHPLevel1) {
    Player level1_warrior{
        5,  "NewbieWarrior", PlayerRace::HUMAN, PlayerClass::WARRIOR, 1, 100, 80, 10, 8, 16,
        15, position};
    int max_hp = GameFormulas::calculate_max_hp(level1_warrior);
    EXPECT_EQ(max_hp, 22);
}

TEST_F(GameFormulasTest, CalculateMaxHPLevel10) {
    Player level10_warrior{
        6,  "VeteranWarrior", PlayerRace::HUMAN, PlayerClass::WARRIOR, 10, 100, 80, 10, 8, 16,
        15, position};
    int max_hp = GameFormulas::calculate_max_hp(level10_warrior);
    EXPECT_EQ(max_hp, 225);
}

TEST_F(GameFormulasTest, CalculateMaxManaMage) {
    int max_mana = GameFormulas::calculate_max_mana(human_mage);
    EXPECT_EQ(max_mana, 120);
}

TEST_F(GameFormulasTest, CalculateMaxManaWarriorIsZero) {
    int max_mana = GameFormulas::calculate_max_mana(human_warrior);
    EXPECT_EQ(max_mana, 0);
}

TEST_F(GameFormulasTest, CalculateMaxManaCleric) {
    int max_mana = GameFormulas::calculate_max_mana(elf_cleric);
    EXPECT_EQ(max_mana, 134);
}

TEST_F(GameFormulasTest, CalculateMaxManaPaladin) {
    int max_mana = GameFormulas::calculate_max_mana(dwarf_paladin);
    EXPECT_EQ(max_mana, 39);
}

TEST_F(GameFormulasTest, CalculateMaxManaLevel1) {
    Player level1_mage{7,  "NewbieMage", PlayerRace::HUMAN, PlayerClass::MAGE, 1, 100, 80, 10, 8,
                       16, 15,           position};
    int max_mana = GameFormulas::calculate_max_mana(level1_mage);
    EXPECT_EQ(max_mana, 24);
}

TEST_F(GameFormulasTest, CalculateMaxManaLevel10) {
    Player level10_mage{
        8, "VeteranMage", PlayerRace::ELF, PlayerClass::MAGE, 10, 100, 80, 10, 8, 16, 15, position};
    int max_mana = GameFormulas::calculate_max_mana(level10_mage);
    EXPECT_EQ(max_mana, 336);
}

TEST_F(GameFormulasTest, CalculateMaxGoldLevel1) {
    Player level1{9,  "Newbie", PlayerRace::HUMAN, PlayerClass::MAGE, 1, 100, 80, 10, 8,
                  16, 15,       position};
    uint64_t max_gold = GameFormulas::calculate_max_gold(level1);
    EXPECT_EQ(max_gold, 100u);
}

TEST_F(GameFormulasTest, CalculateMaxGoldLevel5) {
    Player level5{10, "Explorer", PlayerRace::HUMAN, PlayerClass::MAGE, 5, 100, 80, 10, 8,
                  16, 15,         position};
    uint64_t max_gold = GameFormulas::calculate_max_gold(level5);
    EXPECT_GT(max_gold, 580u);
    EXPECT_LT(max_gold, 600u);
}

TEST_F(GameFormulasTest, CalculateMaxGoldLevel10) {
    Player level10{11, "Veteran", PlayerRace::HUMAN, PlayerClass::MAGE, 10, 100, 80, 10, 8,
                   16, 15,        position};
    uint64_t max_gold = GameFormulas::calculate_max_gold(level10);
    EXPECT_GT(max_gold, 1200u);
    EXPECT_LT(max_gold, 1300u);
}

TEST_F(GameFormulasTest, CalculateMaxGoldLevel20) {
    Player level20{12, "Legend", PlayerRace::HUMAN, PlayerClass::MAGE, 20, 100, 80, 10, 8,
                   16, 15,       position};
    uint64_t max_gold = GameFormulas::calculate_max_gold(level20);
    EXPECT_GT(max_gold, 2600u);
    EXPECT_LT(max_gold, 2800u);
}

TEST_F(GameFormulasTest, MaxGoldIncreaseWithLevel) {
    Player level1{13, "L1", PlayerRace::HUMAN, PlayerClass::MAGE, 1, 100, 80, 10, 8,
                  16, 15,   position};
    Player level10{14, "L10", PlayerRace::HUMAN, PlayerClass::MAGE, 10, 100, 80, 10, 8,
                   16, 15,    position};

    EXPECT_LT(GameFormulas::calculate_max_gold(level1), GameFormulas::calculate_max_gold(level10));
}

TEST_F(GameFormulasTest, MaxHPIncreaseWithLevel) {
    Player level1_warrior{
        15, "L1Warrior", PlayerRace::HUMAN, PlayerClass::WARRIOR, 1, 100, 80, 10, 8,
        16, 15,          position};
    Player level10_warrior{
        16, "L10Warrior", PlayerRace::HUMAN, PlayerClass::WARRIOR, 10, 100, 80, 10, 8,
        16, 15,           position};

    EXPECT_LT(GameFormulas::calculate_max_hp(level1_warrior),
              GameFormulas::calculate_max_hp(level10_warrior));
}

TEST_F(GameFormulasTest, WarriorHasManaZero) {
    for (int i = 1; i <= 10; ++i) {
        Player warrior{static_cast<uint32_t>(100 + i),
                       "Warrior",
                       PlayerRace::HUMAN,
                       PlayerClass::WARRIOR,
                       i,
                       100,
                       80,
                       10,
                       8,
                       16,
                       15,
                       position};
        EXPECT_EQ(GameFormulas::calculate_max_mana(warrior), 0);
    }
}

TEST_F(GameFormulasTest, NonWarriorsHaveMana) {
    Player mage{17, "Mage", PlayerRace::HUMAN, PlayerClass::MAGE, 5, 100, 80, 10, 8,
                16, 15,     position};
    Player cleric{18, "Cleric", PlayerRace::HUMAN, PlayerClass::CLERIC, 5, 100, 80, 10, 8,
                  16, 15,       position};
    Player paladin{19, "Paladin", PlayerRace::HUMAN, PlayerClass::PALADIN, 5, 100, 80, 10, 8,
                   16, 15,        position};

    EXPECT_GT(GameFormulas::calculate_max_mana(mage), 0);
    EXPECT_GT(GameFormulas::calculate_max_mana(cleric), 0);
    EXPECT_GT(GameFormulas::calculate_max_mana(paladin), 0);
}

TEST_F(GameFormulasTest, ElfMageHasMoreManaThanHumanMage) {
    Player human_mage{20, "HumanMage", PlayerRace::HUMAN, PlayerClass::MAGE, 5, 100, 80, 10, 8,
                      16, 15,          position};
    Player elf_mage{21, "ElfMage", PlayerRace::ELF, PlayerClass::MAGE, 5, 100, 80, 10, 8,
                    16, 15,        position};

    EXPECT_LT(GameFormulas::calculate_max_mana(human_mage),
              GameFormulas::calculate_max_mana(elf_mage));
}

TEST_F(GameFormulasTest, DwarfHasMoreHPThanElf) {
    Player elf_warrior{22, "ElfWarrior", PlayerRace::ELF, PlayerClass::WARRIOR, 5, 100, 80, 10, 8,
                       16, 15,           position};
    Player dwarf_warrior{
        23, "DwarfWarrior", PlayerRace::DWARF, PlayerClass::WARRIOR, 5, 100, 80, 10, 8, 16,
        15, position};

    EXPECT_LT(GameFormulas::calculate_max_hp(elf_warrior),
              GameFormulas::calculate_max_hp(dwarf_warrior));
}

TEST_F(GameFormulasTest, CalculateLevelUpLimit) {
    EXPECT_EQ(GameFormulas::calculate_level_up_limit(1), 1000);
    EXPECT_GT(GameFormulas::calculate_level_up_limit(10), 1000);
}

TEST_F(GameFormulasTest, CalculateBaseStats) {
    BaseStats stats = GameFormulas::calculate_base_stats(PlayerRace::HUMAN, PlayerClass::MAGE);
    EXPECT_EQ(stats.strength, 10 + 0);      // fuerza humana + bono de mago
    EXPECT_EQ(stats.intelligence, 10 + 4);  // int humana + bono de mago
}

TEST_F(GameFormulasTest, HealthRecovery) {
    int regen = GameFormulas::calculate_health_recovery(human_mage, 1.0f);
    EXPECT_EQ(regen, 1);
}

TEST_F(GameFormulasTest, TimeManaRecovery) {
    int regen = GameFormulas::calculate_time_mana_recovery(human_mage, 1.0f);
    EXPECT_EQ(regen, 1);
}

TEST_F(GameFormulasTest, MeditationManaRecovery) {
    // human_mage está inicializado en el test con inteligencia 16.
    // factor de meditación mago = 1.5
    int regen = GameFormulas::calculate_meditation_mana_recovery(human_mage, 1.0f);
    EXPECT_EQ(regen, 24);  // 1.5 * 16 * 1.0
}

TEST_F(GameFormulasTest, CanAttackByLevel) {
    // Nivel máximo newbie = 12, max diferencia = 10
    EXPECT_FALSE(GameFormulas::can_attack_by_level(5, 10));   // ambos newbies
    EXPECT_FALSE(GameFormulas::can_attack_by_level(15, 10));  // target newbie
    EXPECT_FALSE(GameFormulas::can_attack_by_level(10, 15));  // attacker newbie
    EXPECT_TRUE(GameFormulas::can_attack_by_level(15, 20));   // diferencia 5 (válido)
    EXPECT_FALSE(GameFormulas::can_attack_by_level(15, 30));  // diferencia 15 (inválido)
}

TEST_F(GameFormulasTest, CalculateAttackExperienceGain) {
    // daño = fuerza * (combate desarmado = fuerza)
    // human_mage fuerza = 10 -> daño = 10 * 10 = 100.
    // atacante nivel = 5, target nivel = 5 -> dif = max(5 - 5 + 10, 0) = 10
    // exp = 100 * 10 = 1000
    int exp = GameFormulas::calculate_attack_experience_gain(human_mage, human_warrior);
    EXPECT_EQ(exp, 1000);
}

TEST_F(GameFormulasTest, CalculateKillExperienceGain) {
    // human_warrior max_hp = 112
    // factor de diferencia de nivel = max(5 - 5 + 10, 0) = 10
    // random float entre 0.0 y 0.1
    int exp = GameFormulas::calculate_kill_experience_gain(human_mage, human_warrior);
    EXPECT_GE(exp, 0);
    EXPECT_LE(exp, 112);  // 0.1 * 1120 = 112
}

TEST_F(GameFormulasTest, CalculateDefenseUnarmed) {
    int def = GameFormulas::calculate_defense(human_warrior);
    EXPECT_EQ(def, 0);  // No tiene items equipados
}
