#ifndef CATALOG_UPDATE_H
#define CATALOG_UPDATE_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "common/updates/game_update.h"

class CatalogUpdate: public GameUpdate {
 private:
    uint32_t player_id;
    std::vector<std::string> catalog;
    uint64_t gold_in_bank;

 public:
    CatalogUpdate(uint32_t player_id, std::vector<std::string> catalog, uint64_t gold_in_bank = 0):
        player_id(player_id), catalog(std::move(catalog)), gold_in_bank(gold_in_bank) {}

    UpdateType get_type() const override {
        return UpdateType::CATALOG;
    }

    uint32_t get_target_player_id() const override {
        return player_id;
    }

    const std::vector<std::string>& get_catalog() const {
        return catalog;
    }

    uint64_t get_gold_in_bank() const {
        return gold_in_bank;
    }
};

#endif
