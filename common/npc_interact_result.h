#ifndef NPC_INTERACT_RESULT_H
#define NPC_INTERACT_RESULT_H

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

enum class InteractStatus {
    SUCCESS,
    INSUFFICIENT_GOLD,
    INVENTORY_FULL,
    ITEM_NOT_FOUND,
    NOT_ALLOWED,  // comando que ese NPC no soporta
    PLAYER_DEAD,
    PLAYER_NOT_DEAD,  // intentó resucitar estando vivo
    ALREADY_FULL,
    INVALID_AMOUNT,
    OUT_OF_RANGE,
    INVALID_TARGET
};

struct InteractResult {
    InteractStatus status = InteractStatus::SUCCESS;
    std::string item_name;     // qué item se compró/vendió/depositó (puede estar vacío)
    uint64_t gold_amount = 0;  // cuánto oro se movió en la transacción (puede ser 0)
    std::vector<std::string> catalog;  // para cuando se necesite

    // constructores para que Werror no me tire error
    InteractResult() = default;
    explicit InteractResult(InteractStatus status): status(status) {}
    InteractResult(InteractStatus status, std::string item_name):
        status(status), item_name(std::move(item_name)) {}
    InteractResult(InteractStatus status, std::string item_name, uint64_t gold_amount):
        status(status), item_name(std::move(item_name)), gold_amount(gold_amount) {}
};

#endif
