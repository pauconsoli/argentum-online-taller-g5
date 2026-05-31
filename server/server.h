#ifndef SERVER_H
#define SERVER_H

#include <atomic>
#include <cstdint>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "common/socket.h"
#include "common/updates/match_list_update.h" 
#include "game/match.h"
#include "server_ops.h"

class AcceptorThread;
class PlayerConnection;

class Server: public ServerOps {
 private:
    std::string service_name;  

    std::unique_ptr<Socket> listener;
    std::unique_ptr<AcceptorThread> acceptor;

    mutable std::mutex clients_mutex;
    std::list<PlayerConnection*> clients;
    std::unordered_set<std::string> nicks_in_use;

    mutable std::mutex matches_mutex;
    std::unordered_map<uint32_t, std::unique_ptr<Match>> matches;
    std::atomic<uint32_t> next_match_id;
    std::atomic<uint32_t> next_player_id;

    std::atomic<bool> keep_running;

 public:
    explicit Server(const std::string& service_name);

    void run();

    uint32_t login(PlayerConnection& conn, const std::string& nick) override;
    std::vector<MatchInfo> list_matches() override;
    uint32_t create_match(const std::string& name, uint8_t max_players,
                          PlayerConnection& conn) override;
    Match* join_match(uint32_t match_id, PlayerConnection& conn) override;
    void leave_match(PlayerConnection& conn) override;
    void disconnect(PlayerConnection& conn) override;

    void add_client(PlayerConnection* conn);
    void remove_client(PlayerConnection* conn);

    ~Server() override;

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
};

#endif
