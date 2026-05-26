#ifndef GAME_FORMULAS_H
#define GAME_FORMULAS_H

class Player;

class GameFormulas {
 public:
    static int calculate_max_hp(const Player& player);

    static int calculate_max_mana(const Player& player);

    static int calculate_max_gold(const Player& player);
};

#endif
