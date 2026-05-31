#ifndef CLIENT_H
#define CLIENT_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

#include "common/direction.h"
#include "common/queue.h"
#include "common/socket.h"
#include "common/thread.h"
#include "common/updates/game_update.h"

#include "client_protocol.h"

class Client {
 private:
    Socket socket;
    ClientProtocol protocol;

    std::mutex send_mutex;

    Queue<std::unique_ptr<GameUpdate>> received_updates;

    std::unique_ptr<Thread> receiver_thread;

 public:
    Client(const std::string& host, const std::string& port);

    void start();

    void stop();

    void join();

    void do_login(const std::string& nick);
    void do_list_matches();
    void do_create_match(const std::string& name, uint8_t max_players);
    void do_join_match(uint32_t match_id);
    void do_select_race_class(uint8_t race, uint8_t klass);
    void do_move(Direction dir);
    void do_leave_match();
    void do_disconnect();

    Queue<std::unique_ptr<GameUpdate>>& get_received_updates();

    ~Client();

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
};

#endif
