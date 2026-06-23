#include "server/game/npcs/priest.h"

#include <utility>
#include <vector>

#include "server/game/inventory.h"
#include "server/game/items/item_registry.h"
#include "server/game/player.h"

Priest::Priest(uint32_t id, NPCType npc_type, const Position& pos):
    CityNPC(id, "Sacerdote", npc_type, pos) {}

InteractResult Priest::on_heal(Player& player) {

    if (player.is_dead()) {
        return InteractResult{InteractStatus::PLAYER_DEAD};
    }

    bool hp_full = player.get_current_hp() == player.get_max_hp();
    bool mana_full = player.get_current_mana() == player.get_max_mana();

    if (hp_full && mana_full) {
        return InteractResult{InteractStatus::ALREADY_FULL};
    }

    player.heal(player.get_max_hp());
    player.restore_mana(player.get_max_mana());

    return InteractResult{InteractStatus::SUCCESS};
}

InteractResult Priest::on_resurrect(Player& player) {

    if (!player.is_dead()) {
        return InteractResult{InteractStatus::PLAYER_NOT_DEAD};
    }

    player.resurrect();

    return InteractResult{InteractStatus::SUCCESS};
}

InteractResult Priest::on_buy(const std::string& item_name, Player& player) {
    const ItemRegistry& registry = ItemRegistry::get_instance();

    if (player.is_dead()) {
        return InteractResult{InteractStatus::PLAYER_DEAD};
    }

    if (!registry.exists(item_name)) {
        return InteractResult{InteractStatus::ITEM_NOT_FOUND};
    }

    if (!registry.is_potion(item_name) && !registry.is_magic_weapon(item_name)) {
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

    const std::string real_name = item->get_name();
    player.get_inventory().add_item(std::move(item), 1);
    return InteractResult{InteractStatus::SUCCESS, real_name, price};
}

InteractResult Priest::on_list(Player& player, Bank& /*bank*/) {
    if (player.is_dead()) {
        return InteractResult{InteractStatus::PLAYER_DEAD};
    }

    const ItemRegistry& registry = ItemRegistry::get_instance();
    InteractResult result{InteractStatus::SUCCESS};

    auto add_with_price = [&](const std::vector<std::string>& keys) {
        for (const auto& name : keys)
            result.catalog.push_back(name + " - $" + std::to_string(registry.get_price(name)));
    };

    add_with_price(registry.get_potion_keys());
    add_with_price(registry.get_magic_weapon_keys());

    return result;
}
