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
#include "common/updates/player_left_update.h"
#include "common/updates/world_map_update.h"
#include "game/basic_match.h"
#include "game/player.h"
#include "gameloop_thread.h"
#include "player_connection.h"
#include "world/cell.h"
#include "world/world.h"
#include "world/world_map.h"

constexpr char SERVER_STOP_COMMAND = 'q';


Server::Server(const std::string& service_name_,
               std::function<std::unique_ptr<World>()> world_factory_):
    service_name(service_name_),
    clients(),
    nicks_in_use(),
    matches(),
    next_match_id(1),
    next_player_id(1),
    keep_running(false),
    world_factory(std::move(world_factory_)) {}

Server::~Server() = default;

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

    std::lock_guard<std::mutex> lk(matches_mutex);
    for (const auto& [id, m] : matches) {
        if (m->get_name() == name) {
            throw std::invalid_argument("Ya existe un match con el nombre '" + name + "'");
        }
    }

    uint32_t match_id = next_match_id.fetch_add(1);
    auto match = std::make_unique<BasicMatch>(match_id, name, max_players, world_factory());
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
    Match* match = it->second.get();
    World& world = match->get_world();
    Player* player = world.get_player(conn.get_player_id());

    if (player) {
        auto update = std::make_shared<PlayerLeftUpdate>(player->get_id(), player->get_name(),
                                                         player->get_clan_id());
        match->broadcast_update_to_all(update);
    }

    it->second->remove_player(&conn);
}

void Server::push_command_to_match(uint32_t match_id, std::unique_ptr<ClientCommand> cmd) {
    std::lock_guard<std::mutex> lk(matches_mutex);
    auto it = matches.find(match_id);
    if (it == matches.end()) {
        return;  // Match no existe
    }
    it->second->push_command(std::move(cmd));
}

void Server::send_world_map_to(PlayerConnection& conn) {
    uint32_t match_id = conn.get_current_match_id();
    if (match_id == 0)
        return;
    std::lock_guard<std::mutex> lk(matches_mutex);
    auto it = matches.find(match_id);
    if (it == matches.end())
        return;
    const WorldMap& m = it->second->get_world().get_map();
    const int w = m.get_width();
    const int h = m.get_height();
    std::vector<MapCellData> cells;
    cells.reserve(static_cast<size_t>(w) * static_cast<size_t>(h));
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const Cell& c = m.get_cell(Position{x, y});
            cells.push_back(
                MapCellData{static_cast<uint8_t>(c.get_terrain_type()), c.is_blocking()});
        }
    }
    try {
        conn.enqueue_update(std::make_shared<WorldMapUpdate>(
            static_cast<uint16_t>(w), static_cast<uint16_t>(h), std::move(cells)));
    } catch (const ClosedQueue&) {
        // El cliente se desconectó.
    }
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
    if (conn->get_state() !=
        PlayerConnection::State::DISCONNECTING) {  // si el cliente se desconectó inesperadamente
                                                   // sin pasar por disconnect()
        const std::string& nick = conn->get_nick();
        if (!nick.empty()) {
            nicks_in_use.erase(nick);
        }
    }
}

// para evitar deadlocks, se obtiene una copia de los punteros a los matches y luego se llama a la
// función pasada por parámetro sin mantener el lock (esto es necesario porque la función pasada por
// parámetro puede querer obtener el lock de matches_mutex, por ejemplo para llamar a list_matches())
void Server::for_each_match(std::function<void(Match&)> fn) {
    std::vector<Match*> active_matches;
    {
        std::lock_guard<std::mutex> lk(matches_mutex);
        active_matches.resize(matches.size());
        std::transform(matches.begin(), matches.end(), active_matches.begin(),
                       [](const auto& kv) { return kv.second.get(); });
    }
    for (Match* m : active_matches) {
        fn(*m);
    }
}

void Server::run() {
    try {
        Socket server_socket(service_name.c_str());

        keep_running = true;

        AcceptorThread acceptor(server_socket, *this);
        GameLoopThread gameloop(*this);

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

        gameloop.stop();
        gameloop.join();

        acceptor.stop();
        acceptor.join();

    } catch (const std::exception& e) {
        std::cerr << "[SERVER] Error: " << e.what() << "\n";
        keep_running = false;
        throw;
    }
    std::cerr << "[SERVER] Shutdown complete\n";
}
