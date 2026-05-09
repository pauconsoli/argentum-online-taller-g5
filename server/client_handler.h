#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "../common/queue.h"
#include "../common/socket.h"

#include "receiver_thread.h"
#include "sender_thread.h"


class ClientHandler {
private:
    Socket peer;
    // ...
    ReceiverThread receiver;
    SenderThread sender;

public:
    // ...
    ~ClientHandler() = default;

    void start_threads();
    void stop_threads() noexcept;
    void hard_kill() noexcept;
    void join_threads();

    bool is_dead() const;
};

#endif
