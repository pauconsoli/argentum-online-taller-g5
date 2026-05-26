#ifndef ACCEPTOR_THREAD_H
#define ACCEPTOR_THREAD_H

#include <list>

#include "../common/queue.h"
#include "../common/socket.h"
#include "../common/thread.h"

class PlayerConnection;
class ClientHandler;
class Server;

// cppcheck-suppress noConstructor
class AcceptorThread: public Thread {
 private:
    //...
    Socket& listener;
    std::list<ClientHandler*> clients;
    Server& server;

    // cppcheck-suppress unusedPrivateFunction
    void reap() noexcept;
    void clear() noexcept;

 public:
    ~AcceptorThread();

    void run() override;
    void stop() override;
};

#endif
