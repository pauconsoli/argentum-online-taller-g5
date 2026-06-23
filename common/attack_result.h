#ifndef ATTACK_RESULT_H
#define ATTACK_RESULT_H

#include <cstdint>
#include <string>

enum class AttackStatus {
    SUCCESS,
    NO_MANA,
    OUT_OF_RANGE,
    INVALID_TARGET,
    DEAD,
    SAFE_ZONE,
    FULL_HP,
    CANNOT_HEAL_NPC,
    NEWBIE_PROTECTION,
    LEVEL_DIFFERENCE,
    SAME_CLAN
};

enum class AttackType { NORMAL, RANGED, MAGIC };

struct AttackResult {
    uint32_t attacker_id;
    uint32_t target_id;
    int damage;  // 0 si lo esquiva
    bool evaded;
    bool target_died;
    bool is_critical = false;
    bool is_healing = false;
    int heal_amount = 0;
    AttackStatus status = AttackStatus::SUCCESS;
    AttackType type = AttackType::NORMAL;
    std::string weapon_or_spell_name = "";
    uint32_t target_clan_id = 0;
};

#endif
