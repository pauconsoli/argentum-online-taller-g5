#ifndef BANK_H
#define BANK_H

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "server/game/inventory.h"
#include "server/game/items/item.h"

struct BankSafe {
    std::vector<std::unique_ptr<Item>> items;
    uint64_t gold = 0;
};

class Bank {
 private:
    std::map<uint32_t, BankSafe> safes;  // caja fuerte de cada jugador

    BankSafe& get_or_create_safe(uint32_t player_id);

 public:
    // oro
    bool deposit_gold(uint32_t player_id, uint64_t amount);
    bool withdraw_gold(uint32_t player_id, uint64_t amount);
    uint64_t get_gold_from_safe(uint32_t player_id) const;

    // items
    bool deposit_item(uint32_t player_id, std::unique_ptr<Item> item);
    std::unique_ptr<Item> withdraw_item(uint32_t player_id, const std::string& item_name);

    // listar
    std::vector<std::string> list_items_from_safe(uint32_t player_id) const;
};

#endif
