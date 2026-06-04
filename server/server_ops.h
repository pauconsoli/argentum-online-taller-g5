#ifndef SERVER_OPS_H
#define SERVER_OPS_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "common/updates/match_list_update.h"

class PlayerConnection;
class Match;
class ClientCommand;

class ServerOps {
 public:
    virtual ~ServerOps() = default;

    virtual uint32_t login(PlayerConnection& conn, const std::string& nick) = 0;

    virtual std::vector<MatchInfo> list_matches() = 0;

    virtual uint32_t create_match(const std::string& name, uint8_t max_players,
                                  PlayerConnection& conn) = 0;

    virtual Match* join_match(uint32_t match_id, PlayerConnection& conn) = 0;

    virtual void leave_match(PlayerConnection& conn) = 0;

    virtual void push_command_to_match(uint32_t match_id, std::unique_ptr<ClientCommand> cmd) = 0;

    virtual void disconnect(PlayerConnection& conn) = 0;
};

#endif
