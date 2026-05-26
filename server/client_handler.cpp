#include "client_handler.h"

#include <sys/socket.h>

#include "receiver_thread.h"
#include "sender_thread.h"


void ClientHandler::start_threads() {
    receiver.start();
    sender.start();
}

void ClientHandler::stop_threads() noexcept {
    receiver.stop();
    sender.stop();
}

void ClientHandler::join_threads() {
    receiver.join();
    sender.join();
}

void ClientHandler::hard_kill() noexcept {
    stop_threads();
    try {
        peer.shutdown(SHUT_RDWR);
        peer.close();
    } catch (const std::exception& e) {
        std::cerr << "[CLIENT_HANDLER] Error closing socket in hard_kill: " << e.what() << "\n";
    }
}

bool ClientHandler::is_dead() const {
    return !receiver.is_alive() && !sender.is_alive();
}
