#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

#include <cstdint>
#include <string>
#include <vector>

#include "../common/socket.h"

class ServerProtocol {
 private:
    Socket& skt;
    // ...

 public:
    explicit ServerProtocol(Socket& socket);
    // ...
};

#endif
