#ifndef ATTACK_RESULT_H
#define ATTACK_RESULT_H

#include <cstdint>

struct AttackResult {
    uint32_t attacker_id;
    uint32_t target_id;
    int damage;  // 0 si lo esquiva
    bool evaded;
    bool target_died;
    bool is_healing = false;
    int heal_amount = 0;
};

#endif
