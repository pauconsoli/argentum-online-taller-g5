#ifndef ACCEPTOR_THREAD_H
#define ACCEPTOR_THREAD_H

#include <list>

#include "common/socket.h"
#include "common/thread.h"

class ClientHandler;
class Server;

class AcceptorThread: public Thread {
 private:
    Socket& listener;
    Server& server;
    std::list<ClientHandler*> clients;

    void reap() noexcept;
    void clear() noexcept;

 public:
    AcceptorThread(Socket& listener_sock, Server& srv);

    void run() override;
    void stop() override;

    ~AcceptorThread();

    AcceptorThread(const AcceptorThread&) = delete;
    AcceptorThread& operator=(const AcceptorThread&) = delete;
};

#endif
