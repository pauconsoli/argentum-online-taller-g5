#ifndef ATTACK_UPDATE_H
#define ATTACK_UPDATE_H

#include <cstdint>

#include "game_update.h"

struct AttackResult {
    uint32_t attacker_id;
    uint32_t target_id;
    int damage;  // 0 si lo esquiva
    bool evaded;
    bool target_died;
};

class AttackUpdate: public GameUpdate {
 private:
    AttackResult result;

 public:
    explicit AttackUpdate(AttackResult result): result(result) {}

    UpdateType get_type() const override {
        return UpdateType::ATTACKED;
    }

    const AttackResult& get_result() const {
        return result;
    }
};

#endif
