#ifndef ATTACK_UPDATE_H
#define ATTACK_UPDATE_H

#include <cstdint>

#include "common/attack_result.h"
#include "game_update.h"

class AttackUpdate: public GameUpdate {
 private:
    uint32_t target_player_id;
    AttackResult result;

 public:
    explicit AttackUpdate(AttackResult result, uint32_t target_player_id = 0):
        target_player_id(target_player_id), result(result) {}

    UpdateType get_type() const override {
        return UpdateType::ATTACKED;
    }

    uint32_t get_target_player_id() const override {
        return target_player_id;
    }

    const AttackResult& get_result() const {
        return result;
    }
};

#endif
