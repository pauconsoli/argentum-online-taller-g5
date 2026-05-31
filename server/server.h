#ifndef SERVER_H
#define SERVER_H

#include <atomic>
#include <cstdint>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "../common/position.h"
#include "../common/queue.h"
#include "../common/thread.h"

class PlayerConnection;
class ClientHandler;
class GameLoopThread;
class AcceptorThread;

class ClientCommand;
class GameUpdate;

class Server {
 private:
    std::string service_name;
    std::list<PlayerConnection*> clients;
    mutable std::mutex clients_mutex;
    std::atomic<bool> keep_running;

    Queue<std::unique_ptr<ClientCommand>> gameloop_command_queue;

 public:
    explicit Server(const std::string& service_name) noexcept;

    void run();

    void add_client(PlayerConnection* client);
    void remove_client(PlayerConnection* client);

    void send_update_to_player(uint32_t player_id, std::shared_ptr<const GameUpdate> update);
    void broadcast_update_to_all(std::shared_ptr<const GameUpdate> update);

    void broadcast_update_to_nearby(const Position& position, int range,
                                    std::shared_ptr<const GameUpdate> update);

    bool is_running() const {
        return keep_running;
    }
    void stop() {
        keep_running = false;
    }

    ~Server() = default;
};

#endif
