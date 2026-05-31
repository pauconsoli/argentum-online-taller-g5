#include "server.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "acceptor_thread.h"
#include "common/queue.h"
#include "common/socket.h"
#include "common/updates/game_update.h"
#include "game/player.h"
#include "gameloop_thread.h"
#include "player_connection.h"

namespace {
constexpr char SERVER_STOP_COMMAND = 'q';  // sacar a config
}


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

void Server::send_update_to_player(uint32_t player_id, std::shared_ptr<const GameUpdate> update) {
    std::unique_lock<std::mutex> lock(clients_mutex);
    auto it =
        std::find_if(clients.begin(), clients.end(), [player_id](const PlayerConnection* client) {
            return client->get_player_id() == player_id;
        });
    if (it != clients.end()) {
        try {
            (*it)->enqueue_message(update);
        } catch (const ClosedQueue&) {}
    }
}

void Server::broadcast_update_to_all(std::shared_ptr<const GameUpdate> update) {
    std::unique_lock<std::mutex> lock(clients_mutex);
    for (auto* client : clients) {
        try {
            client->enqueue_message(update);
        } catch (const ClosedQueue&) {}
    }
}

void Server::broadcast_update_to_nearby(const Position& position, int range,
                                        std::shared_ptr<const GameUpdate> update) {
    std::unique_lock<std::mutex> lock(clients_mutex);
    for (auto* client : clients) {
        if (!client->get_player()) {
            continue;
        }

        Position player_pos = client->get_player()->get_position();
        int distance = std::abs(player_pos.x - position.x) + std::abs(player_pos.y - position.y);

        if (distance <=
            range) {  // range es la distancia máxima para recibir el update de ese evento
            try {
                client->enqueue_message(update);
            } catch (const ClosedQueue&) {}
        }
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
