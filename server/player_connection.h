#ifndef PLAYER_CONNECTION_H
#define PLAYER_CONNECTION_H

#include <cstdint>
#include <memory>

#include "../common/queue.h"

class Player;
class GameUpdate;

class PlayerConnection {
 private:
    uint32_t player_id;
    Player* player;                                        // @not_owned: managed by World
    Queue<std::shared_ptr<const GameUpdate>>& send_queue;  // @not_owned: reference

 public:
    /**
     * Constructs a player connection.
     * @param player_id The unique player ID
     * @param player Pointer to the Player object
     * @param send_queue Reference to queue for sending GameUpdate messages to client
     */
    PlayerConnection(uint32_t player_id, Player* player,
                     Queue<std::shared_ptr<const GameUpdate>>& send_queue);

    uint32_t get_player_id() const;
    Player* get_player();

    /**
     * Enqueues a GameUpdate to be sent to the client.
     * @param update The update to send (shared ownership)
     */
    void enqueue_message(std::shared_ptr<const GameUpdate> update);

    ~PlayerConnection() = default;
};

#endif
