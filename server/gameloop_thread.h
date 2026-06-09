#ifndef GAMELOOP_THREAD_H
#define GAMELOOP_THREAD_H

#include "common/thread.h"

class Server;

class GameLoopThread: public Thread {
 private:
    Server& server;

 public:
    explicit GameLoopThread(Server& server);

    void run() override;
    ~GameLoopThread() = default;
};

#endif
