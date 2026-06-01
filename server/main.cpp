#include <cstdlib>
#include <iostream>
#include <string>

#include "server.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << (argc > 0 ? argv[0] : "taller_server") << " <port>\n";
        return EXIT_FAILURE;
    }

    const std::string port = argv[1];

    try {
        Server server(port);
        server.run();
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Error desconocido\n";
        return EXIT_FAILURE;
    }
}
