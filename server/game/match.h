#ifndef MATCH_H
#define MATCH_H

#include <cstdint>
#include <memory>
#include <string>

#include "common/commands/client_command.h"

class PlayerConnection;
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

    virtual void tick(World& world) = 0;

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void join() = 0;
};

#endif
