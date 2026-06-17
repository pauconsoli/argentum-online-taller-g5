#include "server/game/npcs/merchant.h"

#include <string>
#include <utility>

#include "server/game/game_config.h"
#include "server/game/inventory.h"
#include "server/game/items/item_registry.h"
#include "server/game/player.h"

Merchant::Merchant(uint32_t id, NPCType npc_type, const Position& pos):
    CityNPC(id, "Comerciante", npc_type, pos) {}

InteractResult Merchant::on_buy(const std::string& item_name, Player& player) {
    const ItemRegistry& registry = ItemRegistry::get_instance();

    if (player.is_dead()) {
        return InteractResult{InteractStatus::PLAYER_DEAD};
    }

    if (!registry.exists(item_name)) {
        return InteractResult{InteractStatus::ITEM_NOT_FOUND};
    }

    if (registry.is_magic_weapon(item_name)) {
        return InteractResult{InteractStatus::NOT_ALLOWED};
    }

    uint64_t price = registry.get_price(item_name);
    if (!player.remove_gold(price)) {
        return InteractResult{InteractStatus::INSUFFICIENT_GOLD};
    }

    auto item = registry.create_item(item_name);
    if (!player.get_inventory().can_add_item(*item)) {
        player.add_gold(price);  // revertir el cobro
        return InteractResult{InteractStatus::INVENTORY_FULL};
    }

    player.get_inventory().add_item(std::move(item), 1);
    return InteractResult{InteractStatus::SUCCESS, item_name, price};
}

InteractResult Merchant::on_sell(const std::string& item_name, Player& player) {
    const ItemRegistry& registry = ItemRegistry::get_instance();

    if (registry.is_magic_weapon(item_name)) {
        return InteractResult{InteractStatus::NOT_ALLOWED};
    }
    if (player.is_dead()) {
        return InteractResult{InteractStatus::PLAYER_DEAD};
    }

    auto item = player.get_inventory().remove_item_by_name(item_name);
    if (!item) {
        return InteractResult{InteractStatus::ITEM_NOT_FOUND};
    }
    uint64_t sell_price =
        registry.get_price(item_name) / GameConfig::get_instance().get_npc_buy_item_factor();
    player.add_gold(sell_price);

    return InteractResult{InteractStatus::SUCCESS, item_name, sell_price};
}

// para una primera instancia estoy considerando que todos los merchants venden pociones
InteractResult Merchant::on_list(Player& player, Bank& /*bank*/) {
    if (player.is_dead()) {
        return InteractResult{InteractStatus::PLAYER_DEAD};
    }

    const ItemRegistry& registry = ItemRegistry::get_instance();
    InteractResult result{InteractStatus::SUCCESS};

    const auto& weapons = registry.get_weapon_keys();
    const auto& defensive = registry.get_defensive_keys();
    const auto& potions = registry.get_potion_keys();

    result.catalog.insert(result.catalog.end(), weapons.begin(), weapons.end());
    result.catalog.insert(result.catalog.end(), defensive.begin(), defensive.end());
    result.catalog.insert(result.catalog.end(), potions.begin(), potions.end());

    return result;
}
