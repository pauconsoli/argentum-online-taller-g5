#ifndef SDL_GAME_STATE_H
#define SDL_GAME_STATE_H

#include <cstdint>
#include <map>
#include <vector>

#include "common/updates/inventory_update.h"
#include "common/updates/snapshot_update.h"

class GameState {
 public:
    GameState(uint32_t player_id, uint8_t race, uint8_t klass);

    uint32_t player_id() const;
    uint8_t race() const;
    uint8_t klass() const;
    int player_x() const;
    int player_y() const;
    int hp() const;
    int max_hp() const;
    int mp() const;
    int max_mp() const;
    int level() const;
    uint64_t gold() const;
    uint64_t xp() const;
    bool is_ghost() const;
    uint32_t clan_id() const;
    int selected_slot() const;
    uint32_t selected_npc_id() const;
    const std::map<uint32_t, PlayerSnapshot>& players() const;
    const std::map<uint32_t, NPCSnapshot>& npcs() const;
    const std::vector<NPCSnapshot>& last_npc_snapshot() const;
    const std::vector<GroundItemSnapshot>& ground_items() const;
    const std::vector<InventorySlotData>& inventory_slots() const;

    void apply_snapshot(const SnapshotUpdate& snap, int tile_w, int tile_h);
    void remove_player(uint32_t pid);
    void apply_inventory(const InventoryUpdate& iu);
    void select_npc(uint32_t npc_id);
    void select_slot(int slot);
    void clear_slot_selection();
    void reset_position();

 private:
    uint32_t player_id_;
    uint8_t race_;
    uint8_t klass_;
    int player_x_;
    int player_y_;
    int hp_;
    int max_hp_;
    int mp_;
    int max_mp_;
    int level_;
    uint64_t gold_;
    uint64_t xp_;
    bool is_ghost_;
    uint32_t clan_id_;

    std::map<uint32_t, PlayerSnapshot> players_;
    std::vector<GroundItemSnapshot> ground_items_;
    std::vector<InventorySlotData> inventory_slots_;
    std::map<uint32_t, NPCSnapshot> npcs_;
    std::vector<NPCSnapshot> last_npc_snapshot_;

    int selected_npc_id_;
    int selected_slot_;
};

#endif
