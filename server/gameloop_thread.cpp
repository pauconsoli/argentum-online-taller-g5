#include "gameloop_thread.h"

#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "common/attack_result.h"
#include "common/updates/attack_update.h"
#include "common/updates/death_update.h"
#include "common/updates/inventory_update.h"
#include "common/updates/snapshot_update.h"
#include "game/game_config.h"
#include "game/match.h"
#include "game/player.h"
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

        // OPTIMIZACIÓN DEL GAMELOOP
        using Clock = std::chrono::steady_clock;
        using Ms = std::chrono::duration<double, std::milli>;
        const Ms rate(sleep_ms);

        auto t1 = Clock::now();

        while (should_keep_running()) {

            server.for_each_match([this, tick_seconds, tick_id](Match& match) {
                World& world = match.get_world();
                world.reset_player_movement();
                match.tick();
                auto attack_results = world.update(tick_seconds);

                for (const auto& result : attack_results) {
                    // enviar el resultado del ataque solo al atacante y al objetivo
                    auto update_for_attacker =
                        std::make_shared<AttackUpdate>(result, result.attacker_id);
                    match.send_update_to_player(result.attacker_id, update_for_attacker);
                    auto update_for_target =
                        std::make_shared<AttackUpdate>(result, result.target_id);
                    match.send_update_to_player(result.target_id, update_for_target);
                    if (result.target_died) {
                        auto death_update =
                            std::make_shared<DeathUpdate>(result.target_id, result.attacker_id);
                        match.broadcast_update_to_all(death_update);

                        Player* dead_player = world.get_player(result.target_id);
                        if (dead_player) {
                            std::vector<InventorySlotData> items_data;
                            for (const auto& slot : dead_player->get_inventory().get_slots()) {
                                if (slot.item) {
                                    items_data.push_back({slot.item->get_name(),
                                                          static_cast<uint32_t>(slot.quantity),
                                                          slot.equipped_slot.has_value()});
                                } else {
                                    items_data.push_back({"", 0, false});
                                }
                            }
                            auto inv_update = std::make_shared<InventoryUpdate>(
                                result.target_id, std::move(items_data), dead_player->get_gold());
                            match.send_update_to_player(result.target_id, inv_update);
                        }
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
                    ps.clan_id = p->get_clan_id();
                    ps.direction = static_cast<uint8_t>(p->get_direction());
                    ps.is_moving = p->is_moving();

                    for (const auto& [slot, item] : p->get_equipment()) {
                        if (item) {
                            ps.equipment.push_back(item->get_name());
                        }
                    }

                    snapshots.push_back(ps);
                }

                std::vector<NPCSnapshot> npc_snapshots;
                for (NPC* n : world.get_npcs()) {
                    if (n->is_dead())
                        continue;

                    NPCSnapshot ns;
                    ns.npc_id = n->get_id();
                    ns.name = n->get_name();
                    ns.x = n->get_position().x;
                    ns.y = n->get_position().y;
                    ns.hp = n->get_current_hp();
                    ns.max_hp = n->get_max_hp();
                    ns.is_hostile = n->is_hostile();
                    ns.npc_type = static_cast<uint32_t>(n->get_type());
                    ns.direction = static_cast<uint8_t>(n->get_direction());
                    ns.is_moving = n->is_moving();
                    npc_snapshots.push_back(ns);
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
                    tick_id, std::move(snapshots), std::move(npc_snapshots),
                    std::move(ground_snapshots));
                match.broadcast_update_to_all(snapshot_update);
            });

            tick_id++;

            // OPTIMIZACIÓN DEL GAMELOOP: calculo lo que tardó el tick y sleep solo el tiempo restante
            auto t2 = Clock::now();
            Ms passed = t2 - t1;
            Ms rest = rate - passed;

            if (rest.count() < 0) {
                Ms behind = -rest;  // para que sea +
                Ms skipped_time = Ms(behind.count() - std::fmod(behind.count(), rate.count()));
                uint32_t skipped_ticks = static_cast<uint32_t>(skipped_time.count() / rate.count());

                t1 += std::chrono::duration_cast<Clock::duration>(skipped_time);
                tick_id += skipped_ticks;

                rest = rate - Ms(std::fmod(behind.count(), rate.count()));
            }

            std::this_thread::sleep_for(rest);
            t1 += std::chrono::duration_cast<Clock::duration>(rate);
        }

    } catch (const std::exception& e) {
        std::cerr << "[GAMELOOP] Error: " << e.what() << "\n";
    }

    std::cerr << "[GAMELOOP] Thread finished\n";
}
