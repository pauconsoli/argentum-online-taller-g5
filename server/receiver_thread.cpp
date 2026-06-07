#include "receiver_thread.h"

#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include <sys/socket.h>

#include "common/commands/select_race_class_command.h"
#include "common/liberror.h"
#include "common/protocol_constants.h"
#include "common/updates/error_update.h"
#include "common/updates/login_ok_update.h"
#include "common/updates/match_created_update.h"
#include "common/updates/match_joined_update.h"
#include "common/updates/match_list_update.h"
#include "common/updates/spawned_update.h"
#include "game/match.h"


ReceiverThread::ReceiverThread(Socket& sock, PlayerConnection& conn, ServerOps& ops):
    socket(sock), protocol(sock), player_conn(conn), server_ops(ops) {}

void ReceiverThread::run() {
    try {
        while (should_keep_running()) {
            uint8_t op = protocol.recv_opcode();

            if (op == ClientOpcode::DISCONNECT) {
                break;
            }

            switch (player_conn.get_state()) {
                case PlayerConnection::State::CONNECTED:
                    if (op == ClientOpcode::LOGIN) {
                        handle_login();
                    } else {
                        send_error(ProtocolError::COMMAND_NOT_ALLOWED, "debés loguearte primero");
                    }
                    break;

                case PlayerConnection::State::AUTHENTICATED:
                    switch (op) {
                        case ClientOpcode::LIST_MATCHES:
                            handle_list_matches();
                            break;
                        case ClientOpcode::CREATE_MATCH:
                            handle_create_match();
                            break;
                        case ClientOpcode::JOIN_MATCH:
                            handle_join_match();
                            break;
                        default:
                            send_error(ProtocolError::COMMAND_NOT_ALLOWED,
                                       "comando solo válido en lobby");
                    }
                    break;

                case PlayerConnection::State::IN_MATCH:
                    switch (op) {
                        case ClientOpcode::MOVE:
                            handle_move();
                            break;
                        case ClientOpcode::LEAVE_MATCH:
                            handle_leave_match();
                            break;
                        case ClientOpcode::SELECT_RACE_CLASS:
                            handle_select_race_class();
                            break;
                        default:
                            send_error(ProtocolError::COMMAND_NOT_ALLOWED,
                                       "comando no implementado en este estado");
                    }
                    break;

                case PlayerConnection::State::DISCONNECTING:
                    return;
            }
        }
    } catch (const LibError& e) {
        std::cerr << "[RECEIVER] " << player_conn.get_nick() << ": " << e.what() << "\n";
    } catch (const std::exception& e) {
        std::cerr << "[RECEIVER] Excepción inesperada para " << player_conn.get_nick() << ": "
                  << e.what() << "\n";
    }

    try {
        server_ops.disconnect(player_conn);
    } catch (...) {}

    // para asegurar que el sender thread termine si el cliente se desconectó inesperadamente sin
    // pasar por disconnect()
    try {
        player_conn.close_send_queue();
    } catch (...) {}
}

void ReceiverThread::handle_login() {
    std::string nick = protocol.recv_login_payload();
    try {
        uint32_t pid = server_ops.login(player_conn, nick);
        player_conn.set_state(PlayerConnection::State::AUTHENTICATED);
        player_conn.enqueue_update(std::make_unique<LoginOkUpdate>(pid));
    } catch (const std::exception& e) {
        send_error(ProtocolError::NICK_TAKEN, e.what());
    }
}

void ReceiverThread::handle_list_matches() {
    std::vector<MatchInfo> matches = server_ops.list_matches();
    player_conn.enqueue_update(std::make_unique<MatchListUpdate>(std::move(matches)));
}

void ReceiverThread::handle_create_match() {
    auto payload = protocol.recv_create_match_payload();
    try {
        uint32_t match_id = server_ops.create_match(payload.name, payload.max_players, player_conn);
        player_conn.enqueue_update(std::make_unique<MatchCreatedUpdate>(match_id));
    } catch (const std::exception& e) {
        send_error(ProtocolError::INVALID_ARG, e.what());
    }
}

void ReceiverThread::handle_join_match() {
    uint32_t match_id = protocol.recv_join_match_payload();
    const Match* m = server_ops.join_match(match_id, player_conn);
    if (m == nullptr) {
        send_error(ProtocolError::MATCH_NOT_FOUND, "match no existe o está lleno");
        return;
    }
    player_conn.set_current_match_id(match_id);
    player_conn.set_state(PlayerConnection::State::IN_MATCH);
    player_conn.enqueue_update(
        std::make_unique<MatchJoinedUpdate>(match_id, player_conn.get_player_id()));
}

void ReceiverThread::handle_select_race_class() {
    auto payload = protocol.recv_select_race_class_payload();

    uint32_t match_id = player_conn.get_current_match_id();
    if (match_id == 0) {
        send_error(ProtocolError::COMMAND_NOT_ALLOWED, "no estás en match");
        return;
    }

    auto cmd = std::make_unique<SelectRaceClassCommand>(
        player_conn.get_player_id(), player_conn.get_nick(), payload.race, payload.klass);

    server_ops.push_command_to_match(match_id, std::move(cmd));
}

void ReceiverThread::handle_move() {
    uint32_t match_id = player_conn.get_current_match_id();
    if (match_id == 0) {
        protocol.recv_move_payload(player_conn.get_player_id());
        send_error(ProtocolError::COMMAND_NOT_ALLOWED, "no estás en match");
        return;
    }
    auto cmd = protocol.recv_move_payload(player_conn.get_player_id());
    server_ops.push_command_to_match(match_id, std::move(cmd));
}

void ReceiverThread::handle_leave_match() {
    server_ops.leave_match(player_conn);
    player_conn.set_current_match_id(0);
    player_conn.set_state(PlayerConnection::State::AUTHENTICATED);
}

void ReceiverThread::send_error(uint8_t code, const std::string& detail) {
    player_conn.try_enqueue_update(
        std::make_unique<ErrorUpdate>(player_conn.get_player_id(), code, detail));
}

void ReceiverThread::stop() {
    Thread::stop();
    try {
        socket.shutdown(SHUT_RD);
    } catch (const std::exception& e) {
        std::cerr << "[RECEIVER] Error stopping: " << e.what() << "\n";
    }
}
