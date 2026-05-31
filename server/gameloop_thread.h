#ifndef GAMELOOP_THREAD_H
#define GAMELOOP_THREAD_H

#include <memory>

#include "common/queue.h"
#include "common/thread.h"
#include "world/world.h"

class Server;
class ClientCommand;

class GameLoopThread: public Thread {
 private:
    Queue<std::unique_ptr<ClientCommand>>& gameloop_command_queue;
    std::unique_ptr<World> world;
    Server& server;

 public:
    GameLoopThread(Queue<std::unique_ptr<ClientCommand>>& gameloop_command_queue, Server& server);

    void run() override;
    ~GameLoopThread() = default;
};

#endif
