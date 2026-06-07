#ifndef INVENTORY_UPDATE_H
#define INVENTORY_UPDATE_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "game_update.h"

struct InventorySlotData {
    std::string item_name;
    uint32_t quantity;
    bool is_equipped;
};

class InventoryUpdate: public GameUpdate {
 private:
    uint32_t player_id;
    std::vector<InventorySlotData> items;
    //le agregue [[maybe_unused]] porque no me compila. porque no se usa (Pau)
    [[maybe_unused]] uint64_t gold;  // campo separado, NO va en el inventario
                    // pero como el cliente lo agarra, lo mando junto con inventario

 public:
    InventoryUpdate(uint32_t player_id, std::vector<InventorySlotData> items, uint64_t gold):
        player_id(player_id), items(std::move(items)), gold(gold) {}

    UpdateType get_type() const override {
        return UpdateType::INVENTORY;
    }

    uint32_t get_target_player_id() const override {
        return player_id;
    }

    const std::vector<InventorySlotData>& get_items() const {
        return items;
    }

    uint64_t get_gold() const {
        return gold;
    }
};

#endif
