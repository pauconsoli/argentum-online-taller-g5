#include "gameloop_thread.h"

#include <chrono>
#include <iostream>

#include "game/match.h"
#include "server.h"
#include "world/world.h"

#define GAME_LOOP_SLEEP_MS 33  // ~30 loops/seg aprox


GameLoopThread::GameLoopThread(Server& server, World& world): world(world), server(server) {}

void GameLoopThread::run() {
    try {
        while (should_keep_running()) {
            server.for_each_match([this](Match& match) { match.tick(world); });

            std::this_thread::sleep_for(std::chrono::milliseconds(GAME_LOOP_SLEEP_MS));
        }
    } catch (const std::exception& e) {
        std::cerr << "[GAMELOOP] Error: " << e.what() << "\n";
    }

    std::cerr << "[GAMELOOP] Thread finished\n";
}
