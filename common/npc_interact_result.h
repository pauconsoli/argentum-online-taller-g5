#ifndef NPC_INTERACT_RESULT_H
#define NPC_INTERACT_RESULT_H

#include <cstdint>
#include <string>
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
};

struct InteractResult {
    InteractStatus status = InteractStatus::SUCCESS;
    std::string item_name;     // qué item se compró/vendió/depositó (puede estar vacío)
    uint64_t gold_amount = 0;  // cuánto oro se movió en la transacción (puede ser 0)
    std::vector<std::string> catalog;  // para cuando se necesite
};

#endif
