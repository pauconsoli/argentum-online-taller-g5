#include "server_protocol.h"

#include <cstring>
#include <vector>

#include <arpa/inet.h>

ServerProtocol::ServerProtocol(Socket& socket): skt(socket) {}
