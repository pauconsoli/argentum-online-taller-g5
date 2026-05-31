#include "player_connection.h"

#include <utility>

#include "common/updates/game_update.h"
#include "server/game/player.h"

PlayerConnection::PlayerConnection(uint32_t player_id, Player* player,
                                   Queue<std::unique_ptr<GameUpdate>>& send_queue):
    player_id(player_id), player(player), send_queue(send_queue) {}

uint32_t PlayerConnection::get_player_id() const {
    return player_id;
}

Player* PlayerConnection::get_player() {
    return player;
}

void PlayerConnection::enqueue_message(std::unique_ptr<GameUpdate> update) {
    send_queue.push(std::move(update));
}
