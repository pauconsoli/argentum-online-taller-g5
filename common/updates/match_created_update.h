#ifndef MATCH_CREATED_UPDATE_H
#define MATCH_CREATED_UPDATE_H

#include <cstdint>

#include "game_update.h"

class MatchCreatedUpdate: public GameUpdate {
 public:
    uint32_t match_id;

    explicit MatchCreatedUpdate(uint32_t match_id): match_id(match_id) {}

    UpdateType get_type() const override {
        return UpdateType::MATCH_CREATED;
    }
};

#endif
