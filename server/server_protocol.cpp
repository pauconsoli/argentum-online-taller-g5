#include "server_protocol.h"

#include <cstring>
#include <stdexcept>
#include <utility>
#include <vector>

#include <arpa/inet.h>

#include "common/commands/move_command.h"
#include "common/liberror.h"
#include "common/protocol_constants.h"
#include "common/updates/error_update.h"
#include "common/updates/login_ok_update.h"
#include "common/updates/match_created_update.h"
#include "common/updates/match_joined_update.h"
#include "common/updates/match_list_update.h"
#include "common/updates/move_update.h"
#include "common/updates/player_joined_update.h"
#include "common/updates/player_left_update.h"
#include "common/updates/snapshot_update.h"

ServerProtocol::ServerProtocol(Socket& socket): skt(socket) {}

void ServerProtocol::put_u8(std::vector<uint8_t>& buf, uint8_t v) {
    buf.push_back(v);
}

void ServerProtocol::put_u16(std::vector<uint8_t>& buf, uint16_t v) {
    uint16_t be = htons(v);
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&be);
    buf.insert(buf.end(), p, p + sizeof(be));
}

void ServerProtocol::put_u32(std::vector<uint8_t>& buf, uint32_t v) {
    uint32_t be = htonl(v);
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&be);
    buf.insert(buf.end(), p, p + sizeof(be));
}

void ServerProtocol::put_i32(std::vector<uint8_t>& buf, int32_t v) {
    uint32_t bits;
    std::memcpy(&bits, &v, sizeof(bits));
    put_u32(buf, bits);
}

void ServerProtocol::put_u64(std::vector<uint8_t>& buf, uint64_t v) {
    put_u32(buf, static_cast<uint32_t>(v >> 32));
    put_u32(buf, static_cast<uint32_t>(v & 0xFFFFFFFFULL));
}

void ServerProtocol::put_string(std::vector<uint8_t>& buf, const std::string& s) {
    if (s.size() > UINT16_MAX) {
        throw std::length_error("ServerProtocol::put_string: string too long");
    }
    put_u16(buf, static_cast<uint16_t>(s.size()));
    buf.insert(buf.end(), s.begin(), s.end());
}

// ===========================
// Helpers de deserialización
// ===========================

uint8_t ServerProtocol::recv_u8() {
    uint8_t v;
    if (skt.recvall(&v, 1) == 0) {
        throw LibError(0, "%s", "ServerProtocol::recv_u8: client closed connection");
    }
    return v;
}

uint16_t ServerProtocol::recv_u16() {
    uint8_t b[2] = {};
    if (skt.recvall(b, 2) == 0) {
        throw LibError(0, "%s", "ServerProtocol::recv_u16: client closed connection");
    }
    uint16_t be;
    std::memcpy(&be, b, sizeof(be));
    return ntohs(be);
}

uint32_t ServerProtocol::recv_u32() {
    uint8_t b[4] = {};
    if (skt.recvall(b, 4) == 0) {
        throw LibError(0, "%s", "ServerProtocol::recv_u32: client closed connection");
    }
    uint32_t be;
    std::memcpy(&be, b, sizeof(be));
    return ntohl(be);
}

int32_t ServerProtocol::recv_i32() {
    uint32_t bits = recv_u32();
    int32_t v;
    std::memcpy(&v, &bits, sizeof(v));
    return v;
}

uint64_t ServerProtocol::recv_u64() {
    uint64_t hi = recv_u32();
    uint64_t lo = recv_u32();
    return (hi << 32) | lo;
}

std::string ServerProtocol::recv_string() {
    uint16_t len = recv_u16();
    if (len == 0) {
        return {};
    }
    std::string out(len, '\0');
    if (skt.recvall(out.data(), len) == 0) {
        throw LibError(0, "%s", "ServerProtocol::recv_string: client closed mid-string");
    }
    return out;
}

uint8_t ServerProtocol::recv_opcode() {
    return recv_u8();
}

std::string ServerProtocol::recv_login_payload() {
    return recv_string();
}

ServerProtocol::CreateMatchPayload ServerProtocol::recv_create_match_payload() {
    std::string name = recv_string();
    uint8_t max_players = recv_u8();
    return {std::move(name), max_players};
}

uint32_t ServerProtocol::recv_join_match_payload() {
    return recv_u32();
}

ServerProtocol::RaceClassPayload ServerProtocol::recv_select_race_class_payload() {
    uint8_t race = recv_u8();
    uint8_t klass = recv_u8();
    return {race, klass};
}

std::unique_ptr<ClientCommand> ServerProtocol::recv_move_payload(uint32_t player_id) {
    uint8_t dir_byte = recv_u8();
    if (dir_byte > static_cast<uint8_t>(Direction::RIGHT)) {
        throw LibError(0, "ServerProtocol::recv_move_payload: dirección inválida 0x%02x",
                       static_cast<int>(dir_byte));
    }
    Direction dir = static_cast<Direction>(dir_byte);
    return std::make_unique<MoveCommand>(player_id, dir);
}

void ServerProtocol::send_update(const GameUpdate& update) {
    switch (update.get_type()) {
        case UpdateType::LOGIN_OK:
            send_login_ok(update);
            break;
        case UpdateType::MATCH_LIST:
            send_match_list(update);
            break;
        case UpdateType::MATCH_CREATED:
            send_match_created(update);
            break;
        case UpdateType::MATCH_JOINED:
            send_match_joined(update);
            break;
        case UpdateType::PLAYER_JOINED:
            send_player_joined(update);
            break;
        case UpdateType::PLAYER_LEFT:
            send_player_left(update);
            break;
        case UpdateType::SNAPSHOT:
            send_snapshot(update);
            break;
        case UpdateType::MOVED:
            send_moved(update);
            break;
        case UpdateType::ERROR:
            send_error(update);
            break;
        default:
            throw LibError(0, "ServerProtocol::send_update: UpdateType no implementado: %d",
                           static_cast<int>(update.get_type()));
    }
}

void ServerProtocol::send_login_ok(const GameUpdate& update) {
    const auto& u = static_cast<const LoginOkUpdate&>(update);
    std::vector<uint8_t> buf;
    put_u8(buf, ServerOpcode::LOGIN_OK);
    put_u32(buf, u.player_id);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ServerProtocol::send_login_ok: client closed connection");
    }
}

void ServerProtocol::send_match_list(const GameUpdate& update) {
    const auto& u = static_cast<const MatchListUpdate&>(update);
    if (u.matches.size() > UINT16_MAX) {
        throw std::length_error("send_match_list: demasiadas matches");
    }
    std::vector<uint8_t> buf;
    put_u8(buf, ServerOpcode::MATCH_LIST);
    put_u16(buf, static_cast<uint16_t>(u.matches.size()));
    for (const auto& m : u.matches) {
        put_u32(buf, m.id);
        put_string(buf, m.name);
        put_u8(buf, m.current_players);
        put_u8(buf, m.max_players);
    }
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ServerProtocol::send_match_list: client closed connection");
    }
}

void ServerProtocol::send_match_created(const GameUpdate& update) {
    const auto& u = static_cast<const MatchCreatedUpdate&>(update);
    std::vector<uint8_t> buf;
    put_u8(buf, ServerOpcode::MATCH_CREATED);
    put_u32(buf, u.match_id);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ServerProtocol::send_match_created: client closed connection");
    }
}

void ServerProtocol::send_match_joined(const GameUpdate& update) {
    const auto& u = static_cast<const MatchJoinedUpdate&>(update);
    std::vector<uint8_t> buf;
    put_u8(buf, ServerOpcode::MATCH_JOINED);
    put_u32(buf, u.match_id);
    put_u32(buf, u.your_player_id);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ServerProtocol::send_match_joined: client closed connection");
    }
}

void ServerProtocol::send_player_joined(const GameUpdate& update) {
    const auto& u = static_cast<const PlayerJoinedUpdate&>(update);
    std::vector<uint8_t> buf;
    put_u8(buf, ServerOpcode::PLAYER_JOINED);
    put_u32(buf, u.player_id);
    put_string(buf, u.nick);
    put_u8(buf, u.race);
    put_u8(buf, u.klass);
    put_i32(buf, u.pos.x);
    put_i32(buf, u.pos.y);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ServerProtocol::send_player_joined: client closed connection");
    }
}

void ServerProtocol::send_player_left(const GameUpdate& update) {
    const auto& u = static_cast<const PlayerLeftUpdate&>(update);
    std::vector<uint8_t> buf;
    put_u8(buf, ServerOpcode::PLAYER_LEFT);
    put_u32(buf, u.player_id);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ServerProtocol::send_player_left: client closed connection");
    }
}

void ServerProtocol::send_snapshot(const GameUpdate& update) {
    const auto& u = static_cast<const SnapshotUpdate&>(update);
    if (u.players.size() > UINT16_MAX) {
        throw std::length_error("send_snapshot: demasiados players");
    }
    std::vector<uint8_t> buf;
    put_u8(buf, ServerOpcode::SNAPSHOT);
    put_u32(buf, u.tick);
    put_u16(buf, static_cast<uint16_t>(u.players.size()));
    for (const auto& p : u.players) {
        put_u32(buf, p.player_id);
        put_string(buf, p.nick);
        put_u8(buf, p.race);
        put_u8(buf, p.klass);
        put_i32(buf, p.x);
        put_i32(buf, p.y);
        put_i32(buf, p.hp);
        put_i32(buf, p.max_hp);
        put_i32(buf, p.mp);
        put_i32(buf, p.max_mp);
        put_u64(buf, p.xp);
        put_u64(buf, p.gold);
        put_u16(buf, p.level);
        put_u8(buf, p.is_ghost ? 1 : 0);
    }
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ServerProtocol::send_snapshot: client closed connection");
    }
}

void ServerProtocol::send_moved(const GameUpdate& update) {
    const auto& u = static_cast<const MovedUpdate&>(update);
    std::vector<uint8_t> buf;
    put_u8(buf, ServerOpcode::MOVED);
    put_u32(buf, u.get_player_id());
    put_i32(buf, u.get_pos().x);
    put_i32(buf, u.get_pos().y);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ServerProtocol::send_moved: client closed connection");
    }
}

void ServerProtocol::send_error(const GameUpdate& update) {
    const auto& u = static_cast<const ErrorUpdate&>(update);
    std::vector<uint8_t> buf;
    put_u8(buf, ServerOpcode::ERROR);
    put_u8(buf, u.code);
    put_string(buf, u.detail);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ServerProtocol::send_error: client closed connection");
    }
}
