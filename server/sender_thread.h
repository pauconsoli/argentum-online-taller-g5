#ifndef SENDER_THREAD_H
#define SENDER_THREAD_H

#include "../common/queue.h"
#include "../common/socket.h"
#include "../common/thread.h"

#include "server_protocol.h"

class PlayerConnection;

class SenderThread: public Thread {
private:
    ServerProtocol protocol;
    // ...

public:
    // ...
    void run() override;
    void stop() override;

    ~SenderThread() = default;
};

#endif
