#ifndef GAMELOOP_THREAD_H
#define GAMELOOP_THREAD_H

#include "common/thread.h"

class Server;
class World;

class GameLoopThread: public Thread {
 private:
    World& world;
    Server& server;

 public:
    GameLoopThread(Server& server, World& world);

    void run() override;
    ~GameLoopThread() = default;
};

#endif
