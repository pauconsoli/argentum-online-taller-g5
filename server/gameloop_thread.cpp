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
            server.for_each_match([this](Match& match) { match.tick(world); });  //
            // en cada iteración del game loop se llama a match.tick(world) sobre cada partida
            // activa luego cada match.tick(world) se encarga de procesar los comandos recibidos
            // para esa partida y actualizar el estado de la partida y del mundo en consecuencia

            int sleep_ms = GameConfig::get_instance().get_server_game_loop_sleep_ms();
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
        }
    } catch (const std::exception& e) {
        std::cerr << "[GAMELOOP] Error: " << e.what() << "\n";
    }

    std::cerr << "[GAMELOOP] Thread finished\n";
}
