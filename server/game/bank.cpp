#include "server/game/bank.h"

#include <algorithm>
#include <cctype>
#include <utility>

BankSafe& Bank::get_or_create_safe(uint32_t player_id) {
    return safes[player_id];  // map crea la entrada si no existe
}

bool Bank::deposit_gold(uint32_t player_id, uint64_t amount) {
    if (amount == 0)
        return false;
    get_or_create_safe(player_id).gold += amount;
    return true;
}

bool Bank::withdraw_gold(uint32_t player_id, uint64_t amount) {
    auto it = safes.find(player_id);
    if (it == safes.end() || it->second.gold < amount)
        return false;
    it->second.gold -= amount;
    return true;
}

uint64_t Bank::get_gold_from_safe(uint32_t player_id) const {
    auto it = safes.find(player_id);
    return (it != safes.end()) ? it->second.gold : 0;
}

bool Bank::deposit_item(uint32_t player_id, std::unique_ptr<Item> item) {
    if (!item)
        return false;
    get_or_create_safe(player_id).items.push_back(std::move(item));
    return true;
}

static std::string to_lower(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(), [](unsigned char c) { return std::tolower(c); });
    return r;
}

std::unique_ptr<Item> Bank::withdraw_item(uint32_t player_id, const std::string& item_name) {
    auto it = safes.find(player_id);
    if (it == safes.end())
        return nullptr;

    const std::string name_lower = to_lower(item_name);
    auto& items = it->second.items;
    auto found = std::find_if(items.begin(), items.end(),
                              [&](const auto& i) { return to_lower(i->get_name()) == name_lower; });
    if (found == items.end())
        return nullptr;

    auto item = std::move(*found);
    items.erase(found);
    return item;
}

std::vector<std::string> Bank::list_items_from_safe(uint32_t player_id) const {
    auto it = safes.find(player_id);
    if (it == safes.end())
        return {};

    std::vector<std::string> names;
    for (const auto& item : it->second.items) names.push_back(item->get_name());
    return names;
}
