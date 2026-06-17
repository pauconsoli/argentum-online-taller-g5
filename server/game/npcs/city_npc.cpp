#include "server/game/npcs/city_npc.h"

// estos son valores arbitrarios porque que no afectan el juego por las restrcciones sobre los npc
// de ciudad ver si extraerlos al config, si en algun momento se necesitan para algo
static constexpr int CITY_NPC_HP = 9999;
static constexpr int CITY_NPC_DEFENSE = 9999;
static constexpr int CITY_NPC_LEVEL = 100;


CityNPC::CityNPC(uint32_t id, const std::string& name, NPCType npc_type, const Position& pos):
    NPC(id, name, npc_type, CITY_NPC_LEVEL, CITY_NPC_HP, CITY_NPC_DEFENSE, pos) {}

NPCBehavior CityNPC::update(float /*time*/, const std::vector<Player*>& /*nearby_targets*/) {
    return NPCBehavior{};  // STILL por defecto
}

InteractResult CityNPC::interact(NPCInteraction type, const std::string& arg, int amount,
                                 Player& player, Bank& bank) {
    switch (type) {
        case NPCInteraction::HEAL:
            return on_heal(player);
        case NPCInteraction::RESURRECT:
            return on_resurrect(player);
        case NPCInteraction::BUY:
            return on_buy(arg, player);
        case NPCInteraction::SELL:
            return on_sell(arg, player);
        case NPCInteraction::DEPOSIT_GOLD:
            return on_deposit_gold(amount, player, bank);
        case NPCInteraction::DEPOSIT_ITEM:
            return on_deposit_item(arg, player, bank);
        case NPCInteraction::WITHDRAW_GOLD:
            return on_withdraw_gold(amount, player, bank);
        case NPCInteraction::WITHDRAW_ITEM:
            return on_withdraw_item(arg, player, bank);
        case NPCInteraction::LIST:
            return on_list(player, bank);
    }
    return InteractResult{InteractStatus::NOT_ALLOWED};
}

// DEFAULT y las clases lo sobrreescriben

InteractResult CityNPC::on_heal(Player& /*player*/) {
    return InteractResult{InteractStatus::NOT_ALLOWED};
}

InteractResult CityNPC::on_resurrect(Player& /*player*/) {
    return InteractResult{InteractStatus::NOT_ALLOWED};
}

InteractResult CityNPC::on_buy(const std::string& /*item_name*/, Player& /*player*/) {
    return InteractResult{InteractStatus::NOT_ALLOWED};
}

InteractResult CityNPC::on_sell(const std::string& /*item_name*/, Player& /*player*/) {
    return InteractResult{InteractStatus::NOT_ALLOWED};
}

InteractResult CityNPC::on_deposit_gold(int /*amount*/, Player& /*player*/, Bank& /*bank*/) {
    return InteractResult{InteractStatus::NOT_ALLOWED};
}

InteractResult CityNPC::on_deposit_item(const std::string& /*item_name*/, Player& /*player*/,
                                        Bank& /*bank*/) {
    return InteractResult{InteractStatus::NOT_ALLOWED};
}

InteractResult CityNPC::on_withdraw_gold(int /*amount*/, Player& /*player*/, Bank& /*bank*/) {
    return InteractResult{InteractStatus::NOT_ALLOWED};
}

InteractResult CityNPC::on_withdraw_item(const std::string& /*item_name*/, Player& /*player*/,
                                         Bank& /*bank*/) {
    return InteractResult{InteractStatus::NOT_ALLOWED};
}

InteractResult CityNPC::on_list(Player& /*player*/, Bank& /*bank*/) {
    return InteractResult{InteractStatus::NOT_ALLOWED};
}
