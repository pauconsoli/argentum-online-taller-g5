#ifndef SERVER_UPDATE_HANDLER_H
#define SERVER_UPDATE_HANDLER_H

#include <memory>
#include <string>

#include "common/npc_interact_result.h"
#include "common/queue.h"
#include "common/updates/attack_update.h"
#include "common/updates/catalog_update.h"
#include "common/updates/chat_msg_update.h"
#include "common/updates/clan_result_update.h"
#include "common/updates/clan_review_update.h"
#include "common/updates/death_update.h"
#include "common/updates/error_update.h"
#include "common/updates/game_update.h"
#include "common/updates/inventory_update.h"
#include "common/updates/npc_interact_update.h"
#include "common/updates/player_left_update.h"
#include "common/updates/revive_update.h"
#include "common/updates/snapshot_update.h"
#include "common/updates/system_msg_update.h"
#include "common/updates/world_map_update.h"
#include "sdl_game_state.h"

class ClientMap;

class Notifier {
 public:
    virtual void message(const std::string& text) = 0;
    virtual void play(const std::string& sound, int vol = -1) = 0;
    virtual void show_clan_review(const ClanReviewUpdate& cru) = 0;
    virtual ~Notifier() = default;
};

using UpdateQueue = Queue<std::unique_ptr<GameUpdate>>;

class ServerUpdateHandler {
    GameState& state;
    Notifier& notifier;

 public:
    ServerUpdateHandler(GameState& state, Notifier& notifier);
    void apply_pending(UpdateQueue& queue, ClientMap& map, int tile_w, int tile_h);

 private:
    void on_error(const ErrorUpdate&);
    void on_snapshot(const SnapshotUpdate&, ClientMap&, int tile_w, int tile_h);
    void on_player_left(const PlayerLeftUpdate&);
    static void on_world_map(const WorldMapUpdate&, ClientMap&);
    void on_inventory(const InventoryUpdate&);
    void on_attack(const AttackUpdate&);
    void on_death(const DeathUpdate&, int tile_w, int tile_h);
    void on_chat(const ChatMsgUpdate&);
    void on_catalog(const CatalogUpdate&);
    void on_clan_result(const ClanResultUpdate&);
    void on_clan_review(const ClanReviewUpdate&);
    void on_system_msg(const SystemMsgUpdate&);
    void on_interact(const NpcInteractUpdate&);
    void on_revive(const ReviveUpdate&);
    void show_interact_error(InteractStatus status);
    void play_attack_sound(const AttackResult& r);
};

#endif
