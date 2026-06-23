#ifndef MERCHANT_H
#define MERCHANT_H

#include <string>

#include "server/game/npcs/city_npc.h"

class Merchant: public CityNPC {
 protected:
    InteractResult on_buy(const std::string& item_name, Player& player) override;
    InteractResult on_list(Player& player, Bank& bank) override;
    InteractResult on_sell(const std::string& item_name, Player& player) override;

 public:
    Merchant(uint32_t id, NPCType npc_type, const Position& pos);

    ~Merchant() override = default;
};

#endif
