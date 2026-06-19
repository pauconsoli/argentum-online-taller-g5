#ifndef MATCH_H
#define MATCH_H

#include <cstdint>
#include <memory>
#include <string>

#include "common/commands/client_command.h"

class PlayerConnection;
class GameUpdate;
class World;

class Match {
 public:
    virtual ~Match() = default;

    virtual uint32_t get_id() const = 0;
    virtual const std::string& get_name() const = 0;
    virtual uint8_t get_max_players() const = 0;
    virtual uint8_t get_current_players() const = 0;
    virtual bool is_full() const = 0;

    virtual uint32_t add_player(PlayerConnection* conn) = 0;

    virtual void remove_player(PlayerConnection* conn) = 0;

    virtual void push_command(std::unique_ptr<ClientCommand> cmd) = 0;

    virtual void tick() = 0;

    virtual World& get_world() = 0;

    virtual void stop() = 0;

    virtual void broadcast_update_to_all(std::shared_ptr<const GameUpdate> update) = 0;
    virtual void send_update_to_player(uint32_t player_id,
                                       std::shared_ptr<const GameUpdate> update) = 0;
};

#endif
