#ifndef RECEIVER_THREAD_H
#define RECEIVER_THREAD_H

#include <atomic>
#include <cstdint>
#include <string>

#include "common/socket.h"
#include "common/thread.h"
#include "player_connection.h"
#include "server_ops.h"
#include "server_protocol.h"

class Match;

class ReceiverThread: public Thread {
 private:
    Socket& socket;
    ServerProtocol protocol;
    PlayerConnection& player_conn;
    ServerOps& server_ops;

    std::atomic<Match*> current_match;

    void handle_login();
    void handle_list_matches();
    void handle_create_match();
    void handle_join_match();
    void handle_select_race_class();
    void handle_move();
    void handle_leave_match();

    void send_error(uint8_t code, const std::string& detail);

 public:
    ReceiverThread(Socket& sock, PlayerConnection& conn, ServerOps& ops);

    void run() override;
    void stop() override;

    ~ReceiverThread() = default;
};

#endif
