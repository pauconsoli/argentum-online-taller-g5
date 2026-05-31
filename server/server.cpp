#include "server.h"

#include <iostream>
#include <stdexcept>
#include <utility>

#include "acceptor_thread.h"
#include "player_connection.h"

constexpr char SERVER_STOP_COMMAND = 'q';

Server::Server(const std::string& service_name_):
        service_name(service_name_),
        listener(nullptr),
        acceptor(nullptr),
        clients(),
        nicks_in_use(),
        matches(),
        next_match_id(1),
        next_player_id(1),
        keep_running(false) {}

Server::~Server() = default;

void Server::run() {
    try {
        listener = std::make_unique<Socket>(service_name.c_str());

        keep_running = true;

        acceptor = std::make_unique<AcceptorThread>(*listener, *this);
        acceptor->start();

        std::cerr << "[SERVER] Escuchando en puerto " << service_name
                  << ". Ingresá 'q' para apagar.\n";

        char input;
        while (std::cin.get(input)) {
            if (input == SERVER_STOP_COMMAND) {
                break;
            }
        }

        keep_running = false;
        acceptor->stop();
        acceptor->join();

    } catch (const std::exception& e) {
        std::cerr << "[SERVER] Error fatal: " << e.what() << "\n";
        keep_running = false;
        throw;
    }

    std::cerr << "[SERVER] Apagado completo\n";
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
    for (const auto& kv: matches) {
        const auto& m = kv.second;
        out.push_back({m->get_id(), m->get_name(), m->get_current_players(),
                       m->get_max_players()});
    }
    return out;
}

uint32_t Server::create_match(const std::string& name, uint8_t max_players,
                              PlayerConnection& conn) {
    (void)conn; 
    if (name.empty()) {
        throw std::invalid_argument("nombre de match vacío");
    }
    if (max_players == 0) {
        throw std::invalid_argument("max_players debe ser > 0");
    }
    // TODO (Pau): acá poner tu Match concreto. Por ahora
    // deje una exception para que se pueda detectar en el cliente.
    throw std::runtime_error(
            "create_match: Match concreto no implementado todavía (Pau)");
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
