#ifndef ERROR_UPDATE_H
#define ERROR_UPDATE_H

#include <cstdint>
#include <string>

#include "game_update.h"

class ErrorUpdate: public GameUpdate {
 public:
    uint8_t code;
    std::string detail;

    ErrorUpdate(uint8_t code, std::string detail): code(code), detail(std::move(detail)) {}

    UpdateType get_type() const override {
        return UpdateType::ERROR;
    }
};

#endif
