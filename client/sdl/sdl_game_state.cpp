#include "sdl_game_state.h"

GameState::GameState(uint32_t player_id, uint8_t race, uint8_t klass):
    player_id_(player_id),
    race_(race),
    klass_(klass),
    player_x_(400),
    player_y_(300),
    hp_(100),
    max_hp_(100),
    mp_(100),
    max_mp_(100),
    level_(1),
    gold_(0),
    xp_(0),
    is_ghost_(false),
    clan_id_(0),
    selected_npc_id_(0),
    selected_slot_(-1) {}

uint32_t GameState::player_id() const {
    return player_id_;
}
uint8_t GameState::race() const {
    return race_;
}
uint8_t GameState::klass() const {
    return klass_;
}
int GameState::player_x() const {
    return player_x_;
}
int GameState::player_y() const {
    return player_y_;
}
int GameState::hp() const {
    return hp_;
}
int GameState::max_hp() const {
    return max_hp_;
}
int GameState::mp() const {
    return mp_;
}
int GameState::max_mp() const {
    return max_mp_;
}
int GameState::level() const {
    return level_;
}
uint64_t GameState::gold() const {
    return gold_;
}
uint64_t GameState::xp() const {
    return xp_;
}
bool GameState::is_ghost() const {
    return is_ghost_;
}
uint32_t GameState::clan_id() const {
    return clan_id_;
}
int GameState::selected_slot() const {
    return selected_slot_;
}
uint32_t GameState::selected_npc_id() const {
    return static_cast<uint32_t>(selected_npc_id_);
}
const std::map<uint32_t, PlayerSnapshot>& GameState::players() const {
    return players_;
}
const std::map<uint32_t, NPCSnapshot>& GameState::npcs() const {
    return npcs_;
}
const std::vector<NPCSnapshot>& GameState::last_npc_snapshot() const {
    return last_npc_snapshot_;
}
const std::vector<GroundItemSnapshot>& GameState::ground_items() const {
    return ground_items_;
}
const std::vector<InventorySlotData>& GameState::inventory_slots() const {
    return inventory_slots_;
}

void GameState::apply_snapshot(const SnapshotUpdate& snap, int tile_w, int tile_h) {
    players_.clear();
    for (const auto& ps : snap.players) {
        players_[ps.player_id] = ps;
        if (ps.player_id == player_id_) {
            player_x_ = ps.x * tile_w;
            player_y_ = ps.y * tile_h;
            race_ = ps.race;
            klass_ = ps.klass;
            hp_ = ps.hp;
            mp_ = ps.mp;
            max_hp_ = ps.max_hp;
            max_mp_ = ps.max_mp;
            level_ = ps.level;
            gold_ = ps.gold;
            xp_ = ps.xp;
            is_ghost_ = ps.is_ghost;
            clan_id_ = ps.clan_id;
        }
    }
    ground_items_ = snap.ground_items;
    last_npc_snapshot_ = snap.npcs;
    npcs_.clear();
    for (const auto& ns : snap.npcs) npcs_[ns.npc_id] = ns;
}

void GameState::remove_player(uint32_t pid) {
    players_.erase(pid);
}

void GameState::apply_inventory(const InventoryUpdate& iu) {
    inventory_slots_ = iu.get_items();
    gold_ = iu.get_gold();
}

void GameState::select_npc(uint32_t npc_id) {
    selected_npc_id_ = static_cast<int>(npc_id);
}

void GameState::select_slot(int slot) {
    selected_slot_ = slot;
}

void GameState::clear_slot_selection() {
    selected_slot_ = -1;
}

void GameState::reset_position() {
    player_x_ = -1;
    player_y_ = -1;
}
