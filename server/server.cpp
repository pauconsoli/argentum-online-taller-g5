#include "server.h"

#include <algorithm>
#include <iostream>

#include "acceptor_thread.h"
#include "common/queue.h"
#include "common/socket.h"
#include "gameloop_thread.h"
#include "player_connection.h"

#define SERVER_STOP_COMMAND 'q'


Server::Server(const std::string& service_name) noexcept:
    service_name(service_name), gameloop_command_queue(), keep_running(false) {}

void Server::add_client(PlayerConnection* client) {
    std::unique_lock<std::mutex> lock(clients_mutex);
    clients.push_back(client);
}

void Server::remove_client(PlayerConnection* client) {
    std::unique_lock<std::mutex> lock(clients_mutex);
    auto it = std::find(clients.begin(), clients.end(), client);
    if (it != clients.end()) {
        clients.erase(it);
    }
}

void Server::run() {
    try {
        Socket server_socket(service_name.c_str());

        keep_running = true;

        AcceptorThread acceptor(server_socket, gameloop_command_queue, *this);
        GameLoopThread gameloop(gameloop_command_queue, *this);

        try {
            acceptor.start();
            gameloop.start();
        } catch (const std::exception& e) {
            std::cerr << "[SERVER] Error starting threads: " << e.what() << "\n";
            keep_running = false;
            throw;
        }

        char input;
        while (std::cin.get(input)) {
            if (input == SERVER_STOP_COMMAND) {
                break;
            }
        }

        keep_running = false;
        acceptor.stop();
        gameloop.stop();

        acceptor.join();
        gameloop.join();
    } catch (const std::exception& e) {
        std::cerr << "[SERVER] Error: " << e.what() << "\n";
        keep_running = false;
        throw;
    }
    std::cerr << "[SERVER] Shutdown complete\n";
}
