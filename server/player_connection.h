#ifndef PLAYER_CONNECTION_H
#define PLAYER_CONNECTION_H

#include <cstdint>
#include <memory>

#include "common/queue.h"

class Player;
class GameUpdate;

class PlayerConnection {
 private:
    uint32_t player_id;
    Player* player;
    Queue<std::unique_ptr<GameUpdate>>& send_queue;

 public:
    PlayerConnection(uint32_t player_id, Player* player,
                     Queue<std::unique_ptr<GameUpdate>>& send_queue);

    uint32_t get_player_id() const;
    Player* get_player();
    void enqueue_message(std::unique_ptr<GameUpdate> update);

    ~PlayerConnection() = default;
};

#endif
