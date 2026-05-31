#ifndef SERVER_H
#define SERVER_H

#include <atomic>
#include <cstdint>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "../common/queue.h"
#include "../common/thread.h"

class PlayerConnection;
class ClientHandler;
class GameLoopThread;
class AcceptorThread;
class ClientCommand;

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
    bool is_running() const {
        return keep_running;
    }
    void stop() {
        keep_running = false;
    }

    ~Server() = default;
};

#endif
