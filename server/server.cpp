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
#include "game/inventory.h"
#include "game/items/item.h"
#include "game/player.h"
#include "gameloop_thread.h"
#include "player_connection.h"
#include "world/cell.h"
#include "world/world.h"
#include "world/world_map.h"

constexpr char SERVER_STOP_COMMAND = 'q';


Server::Server(const std::string& service_name_,
               std::function<std::unique_ptr<World>()> world_factory_,
               const std::string& save_path):
    service_name(service_name_),
    clients(),
    nicks_in_use(),
    matches(),
    next_match_id(1),
    next_player_id(1),
    keep_running(false),
    world_factory(std::move(world_factory_)),
    persister(save_path) {
    auto loaded = persister.try_load();

    if (loaded.has_value()) {
        for (const auto& ms: loaded->matches) {
            uint32_t match_id = next_match_id.fetch_add(1);
            auto match = std::make_unique<BasicMatch>(match_id, ms.name, ms.max_players,
                                                      world_factory());
            matches.emplace(match_id, std::move(match));
            std::cerr << "[PERSIST] Match '" << ms.name << "' recreado (max "
                      << static_cast<int>(ms.max_players) << ").\n";
        }
    }
}

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

// void Server::broadcast_update_to_nearby(uint32_t player_id, int range,
//                                         std::shared_ptr<const GameUpdate> update) {
//     Player* origin_player = world->get_player(player_id);
//     if (!origin_player) {
//         return;
//     }

//     Position origin_pos = origin_player->get_position();

//     std::unique_lock<std::mutex> lock(clients_mutex);
//     for (auto* client : clients) {
//         if (client->get_player_id() == 0) {
//             continue;
//         }

//         Player* target_player = world->get_player(client->get_player_id());
//         if (!target_player) {
//             continue;
//         }

//         Position target_pos = target_player->get_position();
//         int distance =
//             std::abs(target_pos.x - origin_pos.x) + std::abs(target_pos.y - origin_pos.y);

//         if (distance <=
//             range) {  // range es la distancia máxima para recibir el update de ese evento
//             try {
//                 client->enqueue_update(update);
//             } catch (const ClosedQueue&) {}
//         }
//     }
// }

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

        std::cerr << "[SERVER] Persistiendo estado final...\n";
        save_state();

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

WorldSnapshot Server::snapshot() {
    WorldSnapshot snap;

    std::vector<Match*> active_matches;
    {
        std::lock_guard<std::mutex> lk(matches_mutex);
        active_matches.reserve(matches.size());
        std::transform(matches.begin(), matches.end(), std::back_inserter(active_matches),
                       [](const auto& kv) { return kv.second.get(); });
    }

    for (Match* match: active_matches) {
        const std::string& match_name = match->get_name();
        World& world = match->get_world();

        // Persistimos también la identidad del match (nombre + capacidad) para
        // poder recrearlo al arrancar el server.
        MatchSave ms;
        ms.name = match_name;
        ms.max_players = match->get_max_players();
        snap.matches.push_back(std::move(ms));

        for (Player* player: world.get_players()) {
            if (player == nullptr)
                continue;

            PlayerSave ps;
            ps.nick = player->get_name();
            ps.match_name = match_name;
            ps.race = static_cast<uint8_t>(player->get_race());
            ps.klass = static_cast<uint8_t>(player->get_class());
            ps.level = static_cast<uint16_t>(player->get_level());
            ps.xp = player->get_experience();
            ps.hp = player->get_current_hp();
            ps.max_hp = player->get_max_hp();
            ps.mp = player->get_current_mana();
            ps.max_mp = player->get_max_mana();
            ps.gold = player->get_gold();
            ps.pos_x = player->get_position().x;
            ps.pos_y = player->get_position().y;

            for (const auto& slot: player->get_inventory().get_slots()) {
                InventorySlotSave saved_slot;
                if (slot.item) {
                    saved_slot.item_name = slot.item->get_name();
                    saved_slot.quantity = static_cast<uint32_t>(slot.quantity);
                    saved_slot.is_equipped = slot.equipped_slot.has_value();
                }
                ps.inventory.push_back(std::move(saved_slot));
            }

            snap.players.push_back(std::move(ps));
        }
    }

    return snap;
}

void Server::save_state() {
    try {
        auto snap = snapshot();
        // Si no hay players, no escribimos: preservamos el save anterior
        // (que SÍ tenía estado válido). Esto pasa típicamente cuando el
        // server arranca y nadie todavía joineó un match, o cuando todos
        // se desconectaron antes del save final.
        if (snap.players.empty()) {
            std::cerr << "[PERSIST] Snapshot vacío, no se escribe save "
                      << "(preservando el archivo anterior).\n";
            return;
        }
        persister.save(snap);
        std::cerr << "[PERSIST] Estado guardado (" << snap.players.size()
                  << " jugador(es)).\n";
    } catch (const std::exception& e) {
        // Loggeamos pero no propagamos: un fallo de IO no debe matar al server.
        std::cerr << "[PERSIST] Error guardando estado: " << e.what() << "\n";
    }
}

std::optional<PlayerSave> Server::find_player_save(const std::string& nick,
                                                   const std::string& match_name) {
    return persister.find_player_save(nick, match_name);
}
