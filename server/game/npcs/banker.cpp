#include "server/game/npcs/banker.h"

#include <string>
#include <utility>

#include "server/game/bank.h"
#include "server/game/player.h"

Banker::Banker(uint32_t id, const Position& pos): CityNPC(id, "Banquero", pos) {}

InteractResult Banker::on_deposit_gold(int amount, Player& player, Bank& bank) {
    if (player.is_dead()) {
        return InteractResult{InteractStatus::PLAYER_DEAD};
    }

    if (amount <= 0) {
        return InteractResult{InteractStatus::INVALID_AMOUNT};
    }

    if (!player.remove_gold(static_cast<uint64_t>(amount))) {
        return InteractResult{InteractStatus::INSUFFICIENT_GOLD};
    }

    bank.deposit_gold(player.get_id(), static_cast<uint64_t>(amount));
    return InteractResult{InteractStatus::SUCCESS, "", static_cast<uint64_t>(amount)};
}

InteractResult Banker::on_deposit_item(const std::string& item_name, Player& player, Bank& bank) {
    if (player.is_dead()) {
        return InteractResult{InteractStatus::PLAYER_DEAD};
    }

    auto item = player.get_inventory().remove_item_by_name(item_name);
    if (!item) {
        return InteractResult{InteractStatus::ITEM_NOT_FOUND};
    }

    bank.deposit_item(player.get_id(), std::move(item));
    return InteractResult{InteractStatus::SUCCESS, item_name};
}

InteractResult Banker::on_withdraw_gold(int amount, Player& player, Bank& bank) {
    if (player.is_dead()) {
        return InteractResult{InteractStatus::PLAYER_DEAD};
    }

    if (amount <= 0) {
        return InteractResult{InteractStatus::INVALID_AMOUNT};
    }

    if (!bank.withdraw_gold(player.get_id(), static_cast<uint64_t>(amount))) {
        return InteractResult{InteractStatus::INSUFFICIENT_GOLD};
    }

    player.add_gold(static_cast<uint64_t>(amount));
    return InteractResult{InteractStatus::SUCCESS, "", static_cast<uint64_t>(amount)};
}

InteractResult Banker::on_withdraw_item(const std::string& item_name, Player& player, Bank& bank) {
    if (player.is_dead()) {
        return InteractResult{InteractStatus::PLAYER_DEAD};
    }

    auto item = bank.withdraw_item(player.get_id(), item_name);
    if (!item) {
        return InteractResult{InteractStatus::ITEM_NOT_FOUND};
    }

    if (!player.get_inventory().can_add_item(*item)) {
        // revertir: devolver el item al banco
        bank.deposit_item(player.get_id(), std::move(item));
        return InteractResult{InteractStatus::INVENTORY_FULL};
    }

    player.get_inventory().add_item(std::move(item), 1);
    return InteractResult{InteractStatus::SUCCESS, item_name};
}

InteractResult Banker::on_list(Player& player, Bank& bank) {
    if (player.is_dead()) {
        return InteractResult{InteractStatus::PLAYER_DEAD};
    }

    InteractResult result{InteractStatus::SUCCESS};

    result.catalog = bank.list_items_from_safe(player.get_id());
    result.gold_amount = bank.get_gold_from_safe(player.get_id());
    return result;
}
