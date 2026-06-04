#include "client_handler.h"

#include <iostream>
#include <utility>

#include <sys/socket.h>

ClientHandler::ClientHandler(Socket&& sock, ServerOps& server_ops):
    peer(std::move(sock)), conn(), receiver(peer, conn, server_ops), sender(peer, conn) {}

void ClientHandler::start_threads() {
    receiver.start();
    sender.start();
}

void ClientHandler::stop_threads() noexcept {
    receiver.stop();
    sender.stop();
}

void ClientHandler::hard_kill() noexcept {
    stop_threads();
    try {
        peer.shutdown(SHUT_RDWR);
    } catch (const std::exception& e) {
        std::cerr << "[CLIENT_HANDLER] Error en hard_kill (shutdown): " << e.what() << "\n";
    }
    try {
        peer.close();
    } catch (const std::exception& e) {
        std::cerr << "[CLIENT_HANDLER] Error en hard_kill (close): " << e.what() << "\n";
    }
}

void ClientHandler::join_threads() {
    receiver.join();
    sender.join();
}

bool ClientHandler::is_dead() const {
    return !receiver.is_alive() && !sender.is_alive();
}

PlayerConnection& ClientHandler::get_player_connection() {
    return conn;
}
