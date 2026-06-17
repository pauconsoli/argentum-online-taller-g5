#ifndef PRIEST_H
#define PRIEST_H

#include <string>

#include "server/game/npcs/city_npc.h"

class Priest: public CityNPC {
 protected:
    InteractResult on_heal(Player& player) override;
    InteractResult on_resurrect(Player& player) override;
    InteractResult on_buy(const std::string& item_name, Player& player) override;
    InteractResult on_list(Player& player, Bank& bank) override;

 public:
    Priest(uint32_t id, NPCType npc_type, const Position& pos);

    ~Priest() override = default;
};

#endif
