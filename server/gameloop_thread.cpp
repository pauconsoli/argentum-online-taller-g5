#include "gameloop_thread.h"

#include <chrono>
#include <iostream>

#include "common/commands/client_command.h"
#include "common/updates/game_update.h"
#include "server.h"

#define GAME_LOOP_SLEEP_MS 16  // ~60 FPS


GameLoopThread::GameLoopThread(Queue<std::unique_ptr<ClientCommand>>& gameloop_command_queue,
                               Server& server):
    gameloop_command_queue(gameloop_command_queue),
    world(std::make_unique<World>(100, 100)),  // cargar de config? ARREGLAR
    server(server) {}

void GameLoopThread::run() {
    try {
        while (should_keep_running()) {
            std::unique_ptr<ClientCommand> cmd;
            while (gameloop_command_queue.try_pop(cmd)) {
                if (cmd) {
                    auto update = cmd->execute(*world);  // ej, move_command.execute(world)
                    if (update) {
                        // TODO(paula): Send update to relevant client(s)
                        // server.send_update(update);
                    }
                }
            }

            // TODO(paula): Update world state (NPC AI, respawns, etc)
            // std::vector<GameUpdate> world_events = world.update();
            // for (const auto& event : world_events) {
            //     server.broadcast_update(event);
            // }

            std::this_thread::sleep_for(std::chrono::milliseconds(GAME_LOOP_SLEEP_MS));
        }
    } catch (const std::exception& e) {
        std::cerr << "[GAMELOOP] Error: " << e.what() << "\n";
    }

    std::cerr << "[GAMELOOP] Thread finished\n";
}
