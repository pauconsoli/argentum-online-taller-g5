#ifndef CLIENT_COMMAND_H
#define CLIENT_COMMAND_H

#include <cstdint>
#include <memory>
#include <vector>

enum class CommandType : uint8_t {
    MOVE = 0x01,
    ATTACK = 0x02,
    MEDITATE = 0x03,
    PICK_UP = 0x04,
    DROP_ITEM = 0x05,
    CHAT = 0x06,
};

class World;
class GameUpdate;

class ClientCommand {
 public:
    virtual uint32_t get_player_id() const = 0;

    virtual std::vector<std::unique_ptr<GameUpdate>> execute(World& world) = 0;
    virtual ~ClientCommand() = default;
};

#endif
