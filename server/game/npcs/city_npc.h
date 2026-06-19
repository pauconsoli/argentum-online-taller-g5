#ifndef CITY_NPC_H
#define CITY_NPC_H

// planteo inicial: uso una especie de template method, cada subclase sobreescribe los métodos que
// le corresponden siendo interact() el método que maneja la interacción

#include <string>
#include <vector>

#include "common/npc_interact_result.h"
#include "common/npc_interaction.h"
#include "server/game/npcs/npc.h"

class Player;
class Bank;

class CityNPC: public NPC {
 protected:
    virtual InteractResult on_heal(Player& player);
    virtual InteractResult on_resurrect(Player& player);
    virtual InteractResult on_buy(const std::string& item_name, Player& player);
    virtual InteractResult on_sell(const std::string& item_name, Player& player);
    virtual InteractResult on_deposit_gold(int amount, Player& player, Bank& bank);
    virtual InteractResult on_deposit_item(const std::string& item_name, Player& player,
                                           Bank& bank);
    virtual InteractResult on_withdraw_gold(int amount, Player& player, Bank& bank);
    virtual InteractResult on_withdraw_item(const std::string& item_name, Player& player,
                                            Bank& bank);
    virtual InteractResult on_list(Player& player, Bank& bank);

 public:
    CityNPC(uint32_t id, const std::string& name, NPCType npc_type, const Position& pos);

    ~CityNPC() override = default;

    // no se mueven ni atacan, pero es extensible si en un futuro necesito agregar npcs de ciudad
    // que hagan algo más
    NPCBehavior update(float time, const std::vector<Player*>& nearby_targets) override;

    bool is_hostile() const override {
        return false;
    }

    // no pueden ser atacados
    bool validate_attack_from(int /*attacker_level*/) const override {
        return false;
    }

    Loot drop_loot() override {
        return Loot{};
    }  // no dropean nada

    InteractResult interact(NPCInteraction type, const std::string& arg, int amount, Player& player,
                            Bank& bank) override;
};

#endif
