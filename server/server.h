#ifndef SERVER_H
#define SERVER_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "common/position.h"
#include "common/queue.h"
#include "common/socket.h"
#include "common/thread.h"
#include "common/updates/match_list_update.h"
#include "game/match.h"
#include "server_ops.h"
#include "world/world.h"

class PlayerConnection;
class ClientHandler;
class AcceptorThread;

class ClientCommand;
class GameUpdate;

class Server: public ServerOps {
 private:
    std::string service_name;

    mutable std::mutex clients_mutex;
    std::list<PlayerConnection*> clients;
    std::unordered_set<std::string> nicks_in_use;

    mutable std::mutex matches_mutex;
    std::unordered_map<uint32_t, std::unique_ptr<Match>> matches;
    std::atomic<uint32_t> next_match_id;
    std::atomic<uint32_t> next_player_id;

    std::atomic<bool> keep_running;

    std::unique_ptr<World> world;

 public:
    explicit Server(const std::string& service_name);

    void run();

    void add_client(PlayerConnection* client);
    void remove_client(PlayerConnection* client);

    void send_update_to_player(uint32_t player_id, std::shared_ptr<const GameUpdate> update);
    void broadcast_update_to_all(std::shared_ptr<const GameUpdate> update);

    void broadcast_update_to_nearby(uint32_t player_id, int range,
                                    std::shared_ptr<const GameUpdate> update);

    void for_each_match(std::function<void(Match&)> fn);

    World& get_world();

    bool is_running() const {
        return keep_running;
    }
    void stop() {
        keep_running = false;
    }

    uint32_t login(PlayerConnection& conn, const std::string& nick) override;
    std::vector<MatchInfo> list_matches() override;
    uint32_t create_match(const std::string& name, uint8_t max_players,
                          PlayerConnection& conn) override;
    Match* join_match(uint32_t match_id, PlayerConnection& conn) override;
    void leave_match(PlayerConnection& conn) override;
    void push_command_to_match(uint32_t match_id, std::unique_ptr<ClientCommand> cmd) override;
    void disconnect(PlayerConnection& conn) override;

    ~Server() override;

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
};

#endif
