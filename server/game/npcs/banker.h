#ifndef BANKER_H
#define BANKER_H

#include <string>

#include "server/game/npcs/city_npc.h"

class Banker: public CityNPC {
 protected:
    InteractResult on_deposit_gold(int amount, Player& player, Bank& bank) override;
    InteractResult on_deposit_item(const std::string& item_name, Player& player,
                                   Bank& bank) override;
    InteractResult on_withdraw_gold(int amount, Player& player, Bank& bank) override;
    InteractResult on_withdraw_item(const std::string& item_name, Player& player,
                                    Bank& bank) override;
    InteractResult on_list(Player& player, Bank& bank) override;

 public:
    Banker(uint32_t id, const Position& pos);
    ~Banker() override = default;
};

#endif
