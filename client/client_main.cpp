// Entry point de DEBUG del cliente — sin Qt ni SDL.
//
// Sirve para smoke-testear el server desde la consola mientras
// feat/qt-lobby no esté listo:
//
//   ./taller_client_cli <host> <port> <nick>
//
// Flujo:
//   1. Conecta al server.
//   2. Manda LOGIN con el nick.
//   3. Imprime updates que llegan (LOGIN_OK, ERROR, ...).
//   4. Comandos por stdin:
//        "list"              -> lista partidas
//        "create <n> <maxp>" -> crea partida
//        "join <id>"         -> se une
//        "move u|d|l|r"      -> mover
//        "leave"             -> salir del match
//        "quit"              -> cerrar
//
// Cuando Qt esté listo, este main puede borrarse o quedar como utilidad de
// debug (renombrar a client_cli_main.cpp y excluir del taller_client final).

#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

#include "client.h"
#include "common/direction.h"
#include "common/queue.h"
#include "common/updates/error_update.h"
#include "common/updates/login_ok_update.h"
#include "common/updates/match_created_update.h"
#include "common/updates/match_joined_update.h"
#include "common/updates/match_list_update.h"
#include "common/updates/move_update.h"
#include "common/updates/player_joined_update.h"
#include "common/updates/player_left_update.h"
#include "common/updates/snapshot_update.h"

namespace {

// Imprime un update legible para humanos. Sirve para debug.
void print_update(const GameUpdate& u) {
    switch (u.get_type()) {
        case UpdateType::LOGIN_OK: {
            const auto& v = static_cast<const LoginOkUpdate&>(u);
            std::cout << "[OK] login. player_id=" << v.player_id << "\n";
            break;
        }
        case UpdateType::MATCH_LIST: {
            const auto& v = static_cast<const MatchListUpdate&>(u);
            std::cout << "[OK] match list (" << v.matches.size() << "):\n";
            for (const auto& m : v.matches) {
                std::cout << "  - id=" << m.id << " name=" << m.name << " ("
                          << static_cast<int>(m.current_players) << "/"
                          << static_cast<int>(m.max_players) << ")\n";
            }
            break;
        }
        case UpdateType::MATCH_CREATED: {
            const auto& v = static_cast<const MatchCreatedUpdate&>(u);
            std::cout << "[OK] match creado. id=" << v.match_id << "\n";
            break;
        }
        case UpdateType::MATCH_JOINED: {
            const auto& v = static_cast<const MatchJoinedUpdate&>(u);
            std::cout << "[OK] entraste al match " << v.match_id
                      << ", tu player_id=" << v.your_player_id << "\n";
            break;
        }
        case UpdateType::PLAYER_JOINED: {
            const auto& v = static_cast<const PlayerJoinedUpdate&>(u);
            std::cout << "[+] entró " << v.nick << " (id=" << v.player_id << ") en (" << v.pos.x
                      << "," << v.pos.y << ")\n";
            break;
        }
        case UpdateType::PLAYER_LEFT: {
            const auto& v = static_cast<const PlayerLeftUpdate&>(u);
            std::cout << "[-] se fue player id=" << v.player_id << "\n";
            break;
        }
        case UpdateType::MOVED: {
            const auto& v = static_cast<const MovedUpdate&>(u);
            std::cout << "[mov] player " << v.get_player_id() << " -> (" << v.get_pos().x << ","
                      << v.get_pos().y << ")\n";
            break;
        }
        case UpdateType::SNAPSHOT: {
            const auto& v = static_cast<const SnapshotUpdate&>(u);
            std::cout << "[snap] tick=" << v.tick << " players=" << v.players.size() << "\n";
            break;
        }
        case UpdateType::ERROR: {
            const auto& v = static_cast<const ErrorUpdate&>(u);
            std::cout << "[ERR] code=" << static_cast<int>(v.code) << " detail=" << v.detail
                      << "\n";
            break;
        }
        default:
            std::cout << "[?] update tipo=" << static_cast<int>(u.get_type()) << "\n";
    }
}

// Thread que vacía la queue de updates y los imprime.
void updates_printer_loop(Client& client, std::atomic<bool>& alive) {
    auto& q = client.get_received_updates();
    while (alive.load()) {
        std::unique_ptr<GameUpdate> u;
        try {
            // try_pop con timeout corto para que el loop pueda salir cuando
            // alive=false aunque no estén llegando updates.
            if (q.try_pop(u)) {
                print_update(*u);
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        } catch (const ClosedQueue&) {
            std::cout << "[INFO] queue cerrada, fin del printer\n";
            return;
        }
    }
}

bool parse_direction(const std::string& s, Direction& out) {
    if (s == "u") {
        out = Direction::UP;
    } else if (s == "d") {
        out = Direction::DOWN;
    } else if (s == "l") {
        out = Direction::LEFT;
    } else if (s == "r") {
        out = Direction::RIGHT;
    } else {
        return false;
    }
    return true;
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Uso: " << (argc > 0 ? argv[0] : "taller_client_cli")
                  << " <host> <port> <nick>\n";
        return 1;
    }

    const std::string host = argv[1];
    const std::string port = argv[2];
    const std::string nick = argv[3];

    try {
        Client client(host, port);
        client.start();

        // Thread que imprime los updates entrantes mientras leemos comandos.
        std::atomic<bool> printer_alive{true};
        std::thread printer(updates_printer_loop, std::ref(client), std::ref(printer_alive));

        // Login automático con el nick pasado por argv.
        client.do_login(nick);

        std::cout << "Comandos: list | create <name> <max> | join <id> | move u|d|l|r | "
                     "leave | quit\n";

        std::string line;
        while (std::getline(std::cin, line)) {
            std::istringstream iss(line);
            std::string cmd;
            iss >> cmd;

            try {
                if (cmd == "list") {
                    client.do_list_matches();
                } else if (cmd == "create") {
                    std::string name;
                    int max = 0;
                    iss >> name >> max;
                    client.do_create_match(name, static_cast<uint8_t>(max));
                } else if (cmd == "join") {
                    uint32_t id = 0;
                    iss >> id;
                    client.do_join_match(id);
                } else if (cmd == "move") {
                    std::string d;
                    iss >> d;
                    Direction dir;
                    if (!parse_direction(d, dir)) {
                        std::cerr << "dirección inválida (u/d/l/r)\n";
                        continue;
                    }
                    client.do_move(dir);
                } else if (cmd == "leave") {
                    client.do_leave_match();
                } else if (cmd == "quit" || cmd == "q") {
                    client.do_disconnect();
                    break;
                } else if (cmd.empty()) {
                    continue;
                } else {
                    std::cerr << "comando desconocido: " << cmd << "\n";
                }
            } catch (const std::exception& e) {
                std::cerr << "[ERROR enviando]: " << e.what() << "\n";
                break;
            }
        }

        printer_alive.store(false);
        client.stop();
        client.join();
        if (printer.joinable()) {
            printer.join();
        }
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
