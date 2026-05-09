#ifndef SERVER_H
#define SERVER_H

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "../common/queue.h"
#include "../common/thread.h"

class GameLoopThread;


class Server {
private:
    //...
    mutable std::mutex clients_mutex;
    std::atomic<bool> keep_running;

public:
    // ...
    void run();

    ~Server() = default;
};

#endif
