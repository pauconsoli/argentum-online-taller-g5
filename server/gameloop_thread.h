#ifndef GAMELOOP_THREAD_H
#define GAMELOOP_THREAD_H

#include "../common/queue.h"
#include "../common/thread.h"


class GameLoopThread: public Thread {
 private:
    //...

 public:
    //...
    void run() override;

    ~GameLoopThread() = default;
};

#endif
