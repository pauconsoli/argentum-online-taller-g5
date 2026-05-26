#include <iostream>
#include <string>

#include "server.h"

int main(int argc, char* argv[]) {
    if (argc != 2)
        return 1;

    // std::string servicename(argv[1]);
    // Server server(servicename);

    try {
        // server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
