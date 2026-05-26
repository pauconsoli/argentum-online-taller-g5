#include <iostream>
#include <string>

#include "../common/socket.h"
#include "client.h"

int main(int argc, char* argv[]) {
    //...

    try {
        //...
        // client.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
