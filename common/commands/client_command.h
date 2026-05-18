#ifndef CLIENT_COMMAND_H
#define CLIENT_COMMAND_H

#include <cstdint>

enum class CommandType : uint8_t {
    MOVE      = 0x01,
    ATTACK    = 0x02,
    MEDITATE  = 0x03,
    PICK_UP   = 0x04,
    DROP_ITEM = 0x05,
    CHAT      = 0x06,
};

class World;  //  Pau deberias implementar esta clase

class ClientCommand {
public:
    virtual void execute(World& world) = 0;
    virtual ~ClientCommand() = default;
};

#endif