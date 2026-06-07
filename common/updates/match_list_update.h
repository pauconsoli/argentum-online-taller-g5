#ifndef MATCH_LIST_UPDATE_H
#define MATCH_LIST_UPDATE_H

#include <cstdint>
#include <string>
#include <vector>

#include "game_update.h"

struct MatchInfo {
    uint32_t id;
    std::string name;
    uint8_t current_players;
    uint8_t max_players;
};

class MatchListUpdate: public GameUpdate {
 public:
    std::vector<MatchInfo> matches;

    explicit MatchListUpdate(std::vector<MatchInfo> matches): matches(std::move(matches)) {}

    UpdateType get_type() const override {
        return UpdateType::MATCH_LIST;
    }
};

#endif
