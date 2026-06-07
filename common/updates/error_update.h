#ifndef ERROR_UPDATE_H
#define ERROR_UPDATE_H

#include <cstdint>
#include <string>
#include <utility>

#include "game_update.h"

class ErrorUpdate: public GameUpdate {
 public:
    uint32_t player_id;
    uint8_t code;
    std::string detail;

    ErrorUpdate(uint32_t player_id, uint8_t code, std::string detail):
        player_id(player_id), code(code), detail(std::move(detail)) {}

    UpdateType get_type() const override {
        return UpdateType::ERROR;
    }

    uint32_t get_target_player_id() const override {
        return player_id;
    }
};

#endif
