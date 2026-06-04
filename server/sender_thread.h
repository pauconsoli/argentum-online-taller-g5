#ifndef SENDER_THREAD_H
#define SENDER_THREAD_H

#include "common/socket.h"
#include "common/thread.h"
#include "player_connection.h"
#include "server_protocol.h"


class SenderThread: public Thread {
 private:
    ServerProtocol protocol;
    PlayerConnection& player_conn;

 public:
    SenderThread(Socket& sock, PlayerConnection& conn): protocol(sock), player_conn(conn) {}

    void run() override;
    void stop() override;

    ~SenderThread() = default;
};

#endif
