#ifndef NPC_INTERACT_UPDATE_H
#define NPC_INTERACT_UPDATE_H

#include <cstdint>
#include <string>
#include <utility>

#include "common/npc_interact_result.h"
#include "common/npc_interaction.h"
#include "common/updates/game_update.h"

class NpcInteractUpdate: public GameUpdate {
 private:
    uint32_t player_id;
    NPCInteraction type;
    InteractResult result;

 public:
    NpcInteractUpdate(uint32_t player_id, NPCInteraction type, InteractResult result):
        player_id(player_id), type(type), result(std::move(result)) {}

    UpdateType get_type() const override {
        return UpdateType::NPC_INTERACT;
    }

    uint32_t get_target_player_id() const override {
        return player_id;
    }

    NPCInteraction get_npc_interaction_type() const {
        return type;
    }

    const InteractResult& get_result() const {
        return result;
    }
};

#endif
