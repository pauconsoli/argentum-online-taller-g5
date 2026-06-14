#include "gameloop_thread.h"

#include <chrono>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "common/updates/chat_message_update.h"
#include "common/updates/snapshot_update.h"
#include "game/game_config.h"
#include "game/match.h"
#include "server.h"
#include "world/world.h"

GameLoopThread::GameLoopThread(Server& server): server(server) {}

void GameLoopThread::run() {
    try {
        uint32_t tick_id = 0;

        int sleep_ms = GameConfig::get_instance().get_server_game_loop_sleep_ms();
        float tick_seconds =
            sleep_ms /
            1000.0f;  // convierto a segundos para usarlo en los cálculos de fórmulas (ver esto)

        while (should_keep_running()) {

            server.for_each_match([this, tick_seconds, tick_id](Match& match) {
                match.tick();

                World& world = match.get_world();
                world.update(tick_seconds);

                auto events = world.pop_events();
                for (const auto& ev : events) {
                    auto msg_update = std::make_shared<ChatMessageUpdate>(ev.target_id, ev.message);
                    if (ev.target_id == 0) {
                        match.broadcast_update_to_all(msg_update);
                    } else {
                        server.send_update_to_player(ev.target_id, msg_update);
                    }
                }

                std::vector<PlayerSnapshot> snapshots;
                for (Player* p : world.get_players()) {
                    PlayerSnapshot ps;
                    ps.player_id = p->get_id();
                    ps.nick = p->get_name();
                    ps.race = static_cast<uint8_t>(p->get_race());
                    ps.klass = static_cast<uint8_t>(p->get_class());
                    ps.x = p->get_position().x;
                    ps.y = p->get_position().y;
                    ps.hp = p->get_current_hp();
                    ps.max_hp = p->get_max_hp();
                    ps.mp = p->get_current_mana();
                    ps.max_mp = p->get_max_mana();
                    ps.xp = p->get_experience();
                    ps.gold = p->get_gold();
                    ps.level = p->get_level();
                    ps.is_ghost = p->is_dead();
                    ps.is_meditating = p->is_meditating();

                    snapshots.push_back(ps);
                }

                std::vector<GroundItemSnapshot> ground_snapshots;
                for (const auto& [pos, ground_item] : world.get_ground_items()) {
                    GroundItemSnapshot world_ground_item;
                    world_ground_item.x = pos.x;
                    world_ground_item.y = pos.y;
                    if (ground_item.gold > 0) {
                        world_ground_item.is_gold = true;
                        world_ground_item.quantity = ground_item.gold;
                    } else if (ground_item.item) {
                        world_ground_item.is_gold = false;
                        world_ground_item.quantity = ground_item.quantity;
                        world_ground_item.name = ground_item.item->get_name();
                    }
                    ground_snapshots.push_back(world_ground_item);
                }

                // esto habría que revisarlo, no se debería enviar de todos, a todos, todas la
                // iteraciones del gameloop, por ahora lo dejo así. podria ser un statsupdate solo
                // de los que cambiaron por ej
                auto snapshot_update = std::make_shared<SnapshotUpdate>(
                    tick_id, std::move(snapshots), std::move(ground_snapshots));
                match.broadcast_update_to_all(snapshot_update);
            });

            tick_id++;

            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
        }
    } catch (const std::exception& e) {
        std::cerr << "[GAMELOOP] Error: " << e.what() << "\n";
    }

    std::cerr << "[GAMELOOP] Thread finished\n";
}
