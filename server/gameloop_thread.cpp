#include "gameloop_thread.h"

#include <chrono>
#include <iostream>

#include "game/game_config.h"
#include "game/match.h"
#include "server.h"
#include "world/world.h"

GameLoopThread::GameLoopThread(Server& server, World& world): world(world), server(server) {}

void GameLoopThread::run() {
    try {
        while (should_keep_running()) {
            server.for_each_match([this](Match& match) { match.tick(world); });

            int sleep_ms = GameConfig::get_instance().get_server_game_loop_sleep_ms();
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
        }
    } catch (const std::exception& e) {
        std::cerr << "[GAMELOOP] Error: " << e.what() << "\n";
    }

    std::cerr << "[GAMELOOP] Thread finished\n";
}
