#include "player_connection.h"

#include <utility>

PlayerConnection::PlayerConnection():
        player_id(0), nick(), current_match_id(0), state(State::CONNECTED), send_queue() {}

void PlayerConnection::set_player_id(uint32_t id) { player_id.store(id); }

void PlayerConnection::set_nick(const std::string& n) { nick = n; }

void PlayerConnection::set_current_match_id(uint32_t match_id) {
    current_match_id.store(match_id);
}

void PlayerConnection::set_state(State new_state) { state.store(new_state); }

uint32_t PlayerConnection::get_player_id() const { return player_id.load(); }

const std::string& PlayerConnection::get_nick() const { return nick; }

uint32_t PlayerConnection::get_current_match_id() const { return current_match_id.load(); }

PlayerConnection::State PlayerConnection::get_state() const { return state.load(); }

void PlayerConnection::enqueue_update(std::unique_ptr<GameUpdate> update) {
    send_queue.push(std::move(update));
}

bool PlayerConnection::try_enqueue_update(std::unique_ptr<GameUpdate> update) {
    return send_queue.try_push(std::move(update));
}

void PlayerConnection::close_send_queue() { send_queue.close(); }
