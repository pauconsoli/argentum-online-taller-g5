#include "basic_match.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "common/commands/client_command.h"
#include "common/updates/game_update.h"
#include "server/player_connection.h"
#include "server/world/world.h"

BasicMatch::BasicMatch(uint32_t match_id, const std::string& name, uint8_t max_players):
    match_id(match_id), name(name), max_players(max_players), current_players(0) {}

uint32_t BasicMatch::get_id() const {
    return match_id;
}

const std::string& BasicMatch::get_name() const {
    return name;
}

uint8_t BasicMatch::get_max_players() const {
    return max_players;
}

uint8_t BasicMatch::get_current_players() const {
    return current_players.load();
}

bool BasicMatch::is_full() const {
    return current_players.load() >= max_players;
}

uint32_t BasicMatch::add_player(PlayerConnection* conn) {
    if (is_full()) {
        throw std::runtime_error("match está lleno");
    }
    std::lock_guard<std::mutex> lk(players_mutex);
    players.push_back(conn);
    current_players.fetch_add(1);
    return conn->get_player_id();
}

void BasicMatch::remove_player(PlayerConnection* conn) {
    std::lock_guard<std::mutex> lk(players_mutex);
    players.remove(conn);
    current_players.fetch_sub(1);
}

void BasicMatch::push_command(std::unique_ptr<ClientCommand> cmd) {
    try {
        command_queue.push(std::move(cmd));
    } catch (const ClosedQueue&) {}
}

// el game loop llama a tick() sobre cada partida activa, y cada partida procesa los comandos
// recibidos para esa partida en su tick()
void BasicMatch::tick(World& world) {
    try {
        std::unique_ptr<ClientCommand> cmd;
        while (command_queue.try_pop(cmd)) {
            if (cmd) {
                auto update = cmd->execute(world);
                if (update) {
                    uint32_t target_id = update->get_target_player_id();
                    auto shared = std::shared_ptr<const GameUpdate>(std::move(update));

                    if (target_id == 0) {
                        broadcast_update_to_all(shared);
                    } else {
                        send_update_to_player(target_id, shared);
                    }
                }
            }
        }
    } catch (const ClosedQueue&) {}
}

void BasicMatch::stop() {
    command_queue.close();
}

void BasicMatch::broadcast_update_to_all(std::shared_ptr<const GameUpdate> update) {
    std::lock_guard<std::mutex> lk(players_mutex);
    for (auto* player_conn : players) {
        try {
            player_conn->enqueue_update(update);
        } catch (const ClosedQueue&) {}
    }
}

void BasicMatch::send_update_to_player(uint32_t player_id,
                                       std::shared_ptr<const GameUpdate> update) {
    std::lock_guard<std::mutex> lk(players_mutex);
    auto it = std::find_if(players.begin(), players.end(), [player_id](PlayerConnection* conn) {
        return conn->get_player_id() == player_id;
    });
    if (it != players.end()) {
        try {
            (*it)->enqueue_update(update);
        } catch (const ClosedQueue&) {}
    }
}
