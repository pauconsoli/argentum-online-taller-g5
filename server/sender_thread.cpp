#include "sender_thread.h"

#include <iostream>
#include <memory>

#include "common/liberror.h"
#include "common/queue.h"
#include "common/updates/game_update.h"

void SenderThread::run() {
    try {
        while (should_keep_running()) {
            std::unique_ptr<GameUpdate> update = player_conn.get_send_queue().pop();
            if (!update) {
                continue;
            }
            protocol.send_update(*update);
        }
    } catch (const ClosedQueue&) {
        // el cliente se desconectó y se cerró .
    } catch (const LibError& e) {
        std::cerr << "[SENDER] Socket error para " << player_conn.get_nick() << ": " << e.what()
                  << "\n";
    } catch (const std::exception& e) {
        std::cerr << "[SENDER] Excepción inesperada: " << e.what() << "\n";
    }
}

void SenderThread::stop() {
    Thread::stop();
    // Cerrar la cola desbloquea el pop() bloqueante del run().
    player_conn.close_send_queue();
}
