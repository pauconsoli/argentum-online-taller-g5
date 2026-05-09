#ifndef RECEIVER_THREAD_H
#define RECEIVER_THREAD_H

#include <string>

#include "../common/queue.h"
#include "../common/socket.h"
#include "../common/thread.h"

#include "server_protocol.h"

class PlayerConnection;

class ReceiverThread: public Thread {
private:
    // ...
    Socket& socket;
    ServerProtocol protocol;
    PlayerConnection& player_conn;

public:
    // ...
    void run() override;
    void stop() override;

    ~ReceiverThread() = default;
};

#endif
