#ifndef BASIC_MATCH_H
#define BASIC_MATCH_H

#include <atomic>
#include <cstdint>
#include <list>
#include <memory>
#include <mutex>
#include <string>

#include "common/queue.h"
#include "match.h"

class PlayerConnection;
class ClientCommand;

class BasicMatch: public Match {
 public:
    BasicMatch(uint32_t match_id, const std::string& name, uint8_t max_players,
               std::unique_ptr<World> world);
    ~BasicMatch() override = default;

    uint32_t get_id() const override;
    const std::string& get_name() const override;
    uint8_t get_max_players() const override;
    uint8_t get_current_players() const override;
    bool is_full() const override;

    uint32_t add_player(PlayerConnection* conn) override;
    void remove_player(PlayerConnection* conn) override;

    void push_command(std::unique_ptr<ClientCommand> cmd) override;

    void tick() override;

    World& get_world() override;

    void stop() override;

    void broadcast_update_to_all(std::shared_ptr<const GameUpdate> update) override;

 private:
    uint32_t match_id;
    std::string name;
    uint8_t max_players;
    std::atomic<uint8_t> current_players;

    mutable std::mutex players_mutex;
    std::list<PlayerConnection*> players;

    Queue<std::unique_ptr<ClientCommand>> command_queue;

    std::unique_ptr<World> world;

    void send_update_to_player(uint32_t player_id, std::shared_ptr<const GameUpdate> update);
};

#endif
