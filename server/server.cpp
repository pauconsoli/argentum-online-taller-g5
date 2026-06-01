#include "server.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "acceptor_thread.h"
#include "common/queue.h"
#include "common/socket.h"
#include "common/updates/game_update.h"
#include "game/basic_match.h"
#include "game/player.h"
#include "gameloop_thread.h"
#include "player_connection.h"
#include "world/world.h"

constexpr char SERVER_STOP_COMMAND = 'q';

Server::Server(const std::string& service_name_):
    service_name(service_name_),
    clients(),
    nicks_in_use(),
    matches(),
    next_match_id(1),
    next_player_id(1),
    keep_running(false),
    world(std::make_unique<World>(100, 100)) {}  // PLACEHOLDER después iría por config

Server::~Server() = default;

World& Server::get_world() {
    return *world;
}

void Server::send_update_to_player(uint32_t player_id, std::shared_ptr<const GameUpdate> update) {
    std::unique_lock<std::mutex> lock(clients_mutex);
    auto it =
        std::find_if(clients.begin(), clients.end(), [player_id](const PlayerConnection* client) {
            return client->get_player_id() == player_id;
        });
    if (it != clients.end()) {
        try {
            (*it)->enqueue_update(update);
        } catch (const ClosedQueue&) {}
    }
}

void Server::broadcast_update_to_all(std::shared_ptr<const GameUpdate> update) {
    std::unique_lock<std::mutex> lock(clients_mutex);
    for (auto* client : clients) {
        try {
            client->enqueue_update(update);
        } catch (const ClosedQueue&) {}
    }
}

void Server::broadcast_update_to_nearby(uint32_t player_id, int range,
                                        std::shared_ptr<const GameUpdate> update) {
    Player* origin_player = world->get_player(player_id);
    if (!origin_player) {
        return;
    }

    Position origin_pos = origin_player->get_position();

    std::unique_lock<std::mutex> lock(clients_mutex);
    for (auto* client : clients) {
        if (client->get_player_id() == 0) {
            continue;
        }

        Player* target_player = world->get_player(client->get_player_id());
        if (!target_player) {
            continue;
        }

        Position target_pos = target_player->get_position();
        int distance =
            std::abs(target_pos.x - origin_pos.x) + std::abs(target_pos.y - origin_pos.y);

        if (distance <=
            range) {  // range es la distancia máxima para recibir el update de ese evento
            try {
                client->enqueue_update(update);
            } catch (const ClosedQueue&) {}
        }
    }
}

uint32_t Server::login(PlayerConnection& conn, const std::string& nick) {
    if (nick.empty()) {
        throw std::invalid_argument("nick vacío");
    }
    std::lock_guard<std::mutex> lk(clients_mutex);
    if (nicks_in_use.count(nick) > 0) {
        throw std::runtime_error("nick ya en uso");
    }
    uint32_t pid = next_player_id.fetch_add(1);
    nicks_in_use.insert(nick);
    conn.set_player_id(pid);
    conn.set_nick(nick);
    return pid;
}

std::vector<MatchInfo> Server::list_matches() {
    std::lock_guard<std::mutex> lk(matches_mutex);
    std::vector<MatchInfo> out;
    out.reserve(matches.size());
    for (const auto& kv : matches) {
        const auto& m = kv.second;
        out.push_back({m->get_id(), m->get_name(), m->get_current_players(), m->get_max_players()});
    }
    return out;
}

uint32_t Server::create_match(const std::string& name, uint8_t max_players,
                              PlayerConnection& conn) {
    (void) conn;
    if (name.empty()) {
        throw std::invalid_argument("nombre de match vacío");
    }
    if (max_players == 0) {
        throw std::invalid_argument("max_players debe ser > 0");
    }
    uint32_t match_id = next_match_id.fetch_add(1);
    auto match = std::make_unique<BasicMatch>(match_id, name, max_players);
    std::lock_guard<std::mutex> lk(matches_mutex);
    matches.emplace(match_id, std::move(match));
    return match_id;
}

Match* Server::join_match(uint32_t match_id, PlayerConnection& conn) {
    std::lock_guard<std::mutex> lk(matches_mutex);
    auto it = matches.find(match_id);
    if (it == matches.end()) {
        return nullptr;
    }
    Match* m = it->second.get();
    if (m->is_full()) {
        return nullptr;
    }
    try {
        m->add_player(&conn);
    } catch (...) {
        return nullptr;
    }
    return m;
}

void Server::leave_match(PlayerConnection& conn) {
    uint32_t match_id = conn.get_current_match_id();
    if (match_id == 0) {
        return;
    }
    std::lock_guard<std::mutex> lk(matches_mutex);
    auto it = matches.find(match_id);
    if (it == matches.end()) {
        return;
    }
    it->second->remove_player(&conn);
    world->remove_player(conn.get_player_id());
}

void Server::push_command_to_match(uint32_t match_id, std::unique_ptr<ClientCommand> cmd) {
    std::lock_guard<std::mutex> lk(matches_mutex);
    auto it = matches.find(match_id);
    if (it == matches.end()) {
        return;  // Match no existe
    }
    it->second->push_command(std::move(cmd));
}

void Server::disconnect(PlayerConnection& conn) {
    if (conn.get_state() == PlayerConnection::State::IN_MATCH) {
        leave_match(conn);
    }

    std::lock_guard<std::mutex> lk(clients_mutex);
    const std::string& nick = conn.get_nick();
    if (!nick.empty()) {
        nicks_in_use.erase(nick);
    }
    conn.set_state(PlayerConnection::State::DISCONNECTING);
    conn.close_send_queue();
}

void Server::add_client(PlayerConnection* conn) {
    std::lock_guard<std::mutex> lk(clients_mutex);
    clients.push_back(conn);
}

void Server::remove_client(PlayerConnection* conn) {
    std::lock_guard<std::mutex> lk(clients_mutex);
    clients.remove(conn);
    const std::string& nick = conn->get_nick();
    if (!nick.empty()) {
        nicks_in_use.erase(nick);
    }
}

void Server::for_each_match(std::function<void(Match&)> fn) {
    std::vector<Match*> snapshot;
    {
        std::lock_guard<std::mutex> lk(matches_mutex);
        snapshot.resize(matches.size());
        std::transform(matches.begin(), matches.end(), snapshot.begin(),
                       [](const auto& kv) { return kv.second.get(); });
    }
    for (Match* m : snapshot) {
        fn(*m);
    }
}

void Server::run() {
    try {
        Socket server_socket(service_name.c_str());

        keep_running = true;

        AcceptorThread acceptor(server_socket, *this);
        GameLoopThread gameloop(*this, *world);

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
