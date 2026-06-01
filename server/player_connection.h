#ifndef PLAYER_CONNECTION_H
#define PLAYER_CONNECTION_H

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>

#include "common/queue.h"
#include "common/updates/game_update.h"

class PlayerConnection {
 public:
    enum class State { CONNECTED, AUTHENTICATED, IN_MATCH, DISCONNECTING };

 private:
    std::atomic<uint32_t> player_id;
    std::string nick;
    std::atomic<uint32_t> current_match_id;
    std::atomic<State> state;
    std::unique_ptr<Queue<std::shared_ptr<const GameUpdate>>> send_queue;

 public:
    PlayerConnection();

    void set_player_id(uint32_t id);
    void set_nick(const std::string& nick);
    void set_current_match_id(uint32_t match_id);
    void set_state(State new_state);

    uint32_t get_player_id() const;
    const std::string& get_nick() const;
    uint32_t get_current_match_id() const;
    State get_state() const;

    void enqueue_update(std::shared_ptr<const GameUpdate> update);
    bool try_enqueue_update(std::shared_ptr<const GameUpdate> update);

    Queue<std::shared_ptr<const GameUpdate>>& get_send_queue() {
        return *send_queue;
    }

    void close_send_queue();

    ~PlayerConnection() = default;

    PlayerConnection(const PlayerConnection&) = delete;
    PlayerConnection& operator=(const PlayerConnection&) = delete;
};

#endif
