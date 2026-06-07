#ifndef DEATH_UPDATE_H
#define DEATH_UPDATE_H

#include <cstdint>

#include "game_update.h"

// por ahora no lo uso. por las dudas, lo dejo

class DeathUpdate: public GameUpdate {
    uint32_t dead_player_id;
    uint32_t killer_id;

 public:
    DeathUpdate(uint32_t dead_id, uint32_t killer_id):
        dead_player_id(dead_id), killer_id(killer_id) {}

    UpdateType get_type() const override {
        return UpdateType::DEATH;
    }
    uint32_t get_dead_id() const {
        return dead_player_id;
    }
    uint32_t get_killer_id() const {
        return killer_id;
    }
};
#endif
