#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <memory>

#include "common/socket.h"
#include "player_connection.h"
#include "receiver_thread.h"
#include "sender_thread.h"
#include "server_ops.h"

class ClientHandler {
 private:
    Socket peer;
    PlayerConnection conn;
    ReceiverThread receiver;
    SenderThread sender;

 public:
    ClientHandler(Socket&& sock, ServerOps& server_ops);

    void start_threads();
    void stop_threads() noexcept;

    void hard_kill() noexcept;

    void join_threads();

    bool is_dead() const;

    PlayerConnection& get_player_connection();

    ~ClientHandler() = default;

    ClientHandler(const ClientHandler&) = delete;
    ClientHandler& operator=(const ClientHandler&) = delete;
};

#endif
