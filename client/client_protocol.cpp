#include "client_protocol.h"

#include <cstring>
#include <stdexcept>
#include <utility>

#include <arpa/inet.h>

#include "common/attack_result.h"
#include "common/liberror.h"
#include "common/protocol_constants.h"
#include "common/updates/attack_update.h"
#include "common/updates/error_update.h"
#include "common/updates/login_ok_update.h"
#include "common/updates/match_created_update.h"
#include "common/updates/match_joined_update.h"
#include "common/updates/match_list_update.h"
#include "common/updates/moved_update.h"
#include "common/updates/player_joined_update.h"
#include "common/updates/player_left_update.h"
#include "common/updates/snapshot_update.h"
#include "common/updates/spawned_update.h"
#include "common/updates/world_map_update.h"

ClientProtocol::ClientProtocol(Socket& socket): skt(socket) {}

void ClientProtocol::put_u8(std::vector<uint8_t>& buf, uint8_t v) {
    buf.push_back(v);
}

void ClientProtocol::put_u16(std::vector<uint8_t>& buf, uint16_t v) {
    uint16_t be = htons(v);
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&be);
    buf.insert(buf.end(), p, p + sizeof(be));
}

void ClientProtocol::put_u32(std::vector<uint8_t>& buf, uint32_t v) {
    uint32_t be = htonl(v);
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&be);
    buf.insert(buf.end(), p, p + sizeof(be));
}

void ClientProtocol::put_i32(std::vector<uint8_t>& buf, int32_t v) {
    uint32_t bits;
    std::memcpy(&bits, &v, sizeof(bits));
    put_u32(buf, bits);
}

void ClientProtocol::put_u64(std::vector<uint8_t>& buf, uint64_t v) {
    put_u32(buf, static_cast<uint32_t>(v >> 32));
    put_u32(buf, static_cast<uint32_t>(v & 0xFFFFFFFFULL));
}

void ClientProtocol::put_string(std::vector<uint8_t>& buf, const std::string& s) {
    if (s.size() > UINT16_MAX) {
        throw std::length_error("ClientProtocol::put_string: string too long");
    }
    put_u16(buf, static_cast<uint16_t>(s.size()));
    buf.insert(buf.end(), s.begin(), s.end());
}

uint8_t ClientProtocol::recv_u8() {
    uint8_t v;
    if (skt.recvall(&v, 1) == 0) {
        throw LibError(0, "%s", "ClientProtocol::recv_u8: server closed connection");
    }
    return v;
}

uint16_t ClientProtocol::recv_u16() {
    uint8_t b[2] = {};
    if (skt.recvall(b, 2) == 0) {
        throw LibError(0, "%s", "ClientProtocol::recv_u16: server closed connection");
    }
    uint16_t be;
    std::memcpy(&be, b, sizeof(be));
    return ntohs(be);
}

uint32_t ClientProtocol::recv_u32() {
    uint8_t b[4] = {};
    if (skt.recvall(b, 4) == 0) {
        throw LibError(0, "%s", "ClientProtocol::recv_u32: server closed connection");
    }
    uint32_t be;
    std::memcpy(&be, b, sizeof(be));
    return ntohl(be);
}

int32_t ClientProtocol::recv_i32() {
    uint32_t bits = recv_u32();
    int32_t v;
    std::memcpy(&v, &bits, sizeof(v));
    return v;
}

uint64_t ClientProtocol::recv_u64() {
    uint64_t hi = recv_u32();
    uint64_t lo = recv_u32();
    return (hi << 32) | lo;
}

std::string ClientProtocol::recv_string() {
    uint16_t len = recv_u16();
    if (len == 0) {
        return {};
    }
    std::string out(len, '\0');
    if (skt.recvall(out.data(), len) == 0) {
        throw LibError(0, "%s", "ClientProtocol::recv_string: server closed mid-string");
    }
    return out;
}

void ClientProtocol::send_login(const std::string& nick) {
    std::vector<uint8_t> buf;
    put_u8(buf, ClientOpcode::LOGIN);
    put_string(buf, nick);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ClientProtocol::send_login: server closed connection");
    }
}

void ClientProtocol::send_list_matches() {
    uint8_t op = ClientOpcode::LIST_MATCHES;
    if (skt.sendall(&op, 1) == 0) {
        throw LibError(0, "%s", "ClientProtocol::send_list_matches: server closed connection");
    }
}

void ClientProtocol::send_create_match(const std::string& match_name, uint8_t max_players) {
    std::vector<uint8_t> buf;
    put_u8(buf, ClientOpcode::CREATE_MATCH);
    put_string(buf, match_name);
    put_u8(buf, max_players);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ClientProtocol::send_create_match: server closed connection");
    }
}

void ClientProtocol::send_join_match(uint32_t match_id) {
    std::vector<uint8_t> buf;
    put_u8(buf, ClientOpcode::JOIN_MATCH);
    put_u32(buf, match_id);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ClientProtocol::send_join_match: server closed connection");
    }
}

void ClientProtocol::send_select_race_class(uint8_t race, uint8_t klass) {
    std::vector<uint8_t> buf;
    put_u8(buf, ClientOpcode::SELECT_RACE_CLASS);
    put_u8(buf, race);
    put_u8(buf, klass);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ClientProtocol::send_select_race_class: server closed connection");
    }
}

void ClientProtocol::send_move(Direction dir) {
    std::vector<uint8_t> buf;
    put_u8(buf, ClientOpcode::MOVE);
    put_u8(buf, static_cast<uint8_t>(dir));
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ClientProtocol::send_move: server closed connection");
    }
}

void ClientProtocol::send_attack(uint32_t target_id) {
    std::vector<uint8_t> buf;
    put_u8(buf, ClientOpcode::ATTACK);
    put_u32(buf, target_id);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ClientProtocol::send_attack: server closed connection");
    }
}

void ClientProtocol::send_meditate() {
    uint8_t op = ClientOpcode::MEDITATE;
    if (skt.sendall(&op, 1) == 0) {
        throw LibError(0, "%s", "ClientProtocol::send_meditate: server closed connection");
    }
}

void ClientProtocol::send_pick_up() {
    uint8_t op = ClientOpcode::PICK_UP;
    if (skt.sendall(&op, 1) == 0) {
        throw LibError(0, "%s", "ClientProtocol::send_pick_up: server closed connection");
    }
}

void ClientProtocol::send_drop_item(uint8_t slot_index) {
    std::vector<uint8_t> buf;
    put_u8(buf, ClientOpcode::DROP_ITEM);
    put_u8(buf, slot_index);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ClientProtocol::send_drop_item: server closed connection");
    }
}

void ClientProtocol::send_disconnect() {
    uint8_t op = ClientOpcode::DISCONNECT;
    if (skt.sendall(&op, 1) == 0) {
        throw LibError(0, "%s", "ClientProtocol::send_disconnect: server closed connection");
    }
}

void ClientProtocol::send_leave_match() {
    uint8_t op = ClientOpcode::LEAVE_MATCH;
    if (skt.sendall(&op, 1) == 0) {
        throw LibError(0, "%s", "ClientProtocol::send_leave_match: server closed connection");
    }
}

std::unique_ptr<GameUpdate> ClientProtocol::receive_update() {
    uint8_t op = recv_u8();
    switch (op) {
        case ServerOpcode::LOGIN_OK:
            return recv_login_ok();
        case ServerOpcode::MATCH_LIST:
            return recv_match_list();
        case ServerOpcode::MATCH_CREATED:
            return recv_match_created();
        case ServerOpcode::MATCH_JOINED:
            return recv_match_joined();
        case ServerOpcode::PLAYER_JOINED:
            return recv_player_joined();
        case ServerOpcode::PLAYER_LEFT:
            return recv_player_left();
        case ServerOpcode::PLAYER_SPAWNED:
            return recv_player_spawned();
        case ServerOpcode::WORLD_MAP:
            return recv_world_map();
        case ServerOpcode::ATTACKED:
            return recv_attacked();
        case ServerOpcode::DEATH:
            return recv_death();
        case ServerOpcode::INVENTORY:
            return recv_inventory();
        case ServerOpcode::SNAPSHOT:
            return recv_snapshot();
        case ServerOpcode::MOVED:
            return recv_moved();
        case ServerOpcode::ERROR:
            return recv_error();
        default:
            throw LibError(0, "ClientProtocol::receive_update: opcode desconocido 0x%02x",
                           static_cast<int>(op));
    }
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_login_ok() {
    uint32_t player_id = recv_u32();
    return std::make_unique<LoginOkUpdate>(player_id);
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_match_list() {
    uint16_t n = recv_u16();
    std::vector<MatchInfo> matches;
    matches.reserve(n);
    for (uint16_t i = 0; i < n; ++i) {
        uint32_t id = recv_u32();
        std::string name = recv_string();
        uint8_t cur = recv_u8();
        uint8_t mx = recv_u8();
        matches.push_back({id, std::move(name), cur, mx});
    }
    return std::make_unique<MatchListUpdate>(std::move(matches));
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_match_created() {
    uint32_t match_id = recv_u32();
    return std::make_unique<MatchCreatedUpdate>(match_id);
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_match_joined() {
    uint32_t match_id = recv_u32();
    uint32_t your_pid = recv_u32();
    return std::make_unique<MatchJoinedUpdate>(match_id, your_pid);
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_player_joined() {
    uint32_t pid = recv_u32();
    std::string nick = recv_string();
    uint8_t race = recv_u8();
    uint8_t klass = recv_u8();
    int32_t x = recv_i32();
    int32_t y = recv_i32();
    return std::make_unique<PlayerJoinedUpdate>(pid, std::move(nick), race, klass, Position{x, y});
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_player_left() {
    uint32_t pid = recv_u32();
    return std::make_unique<PlayerLeftUpdate>(pid);
}


// Cuando Chiari conecte SDL, estos métodos deben construir los GameUpdate
// concretos (AttackUpdate, DeathUpdate, InventoryUpdate).
std::unique_ptr<GameUpdate> ClientProtocol::recv_attacked() {
    AttackResult r;
    r.attacker_id = recv_u32();
    r.target_id = recv_u32();
    r.damage = recv_i32();
    r.evaded = (recv_u8() != 0);
    r.target_died = (recv_u8() != 0);
    return std::make_unique<AttackUpdate>(r);
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_death() {
    recv_u32();  // dead_id
    recv_u32();  // killer_id
    return nullptr;
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_inventory() {
    recv_u32();              // target_player_id
    uint16_t n = recv_u16();
    for (uint16_t i = 0; i < n; ++i) {
        recv_string();  // item_name
        recv_u32();     // quantity
        recv_u8();      // is_equipped
    }
    recv_u64();  // gold
    return nullptr;
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_player_spawned() {
    // TODO Chiari: cuando integres SDL, quizá quieras un SpawnedUpdate
    // verdadero del lado cliente con info adicional. Por ahora basta con
    // que el server pueda mandar el opcode sin que el cliente se queje.
    uint32_t pid = recv_u32();
    std::string nick = recv_string();
    uint8_t race = recv_u8();
    uint8_t klass = recv_u8();
    int32_t x = recv_i32();
    int32_t y = recv_i32();
    return std::make_unique<PlayerJoinedUpdate>(pid, std::move(nick), race, klass, Position{x, y});
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_world_map() {
    uint16_t width = recv_u16();
    uint16_t height = recv_u16();
    const size_t total = static_cast<size_t>(width) * static_cast<size_t>(height);
    std::vector<MapCellData> cells;
    cells.reserve(total);
    for (size_t i = 0; i < total; ++i) {
        MapCellData c;
        c.terrain_type = recv_u8();
        c.blocking = (recv_u8() != 0);
        cells.push_back(c);
    }
    return std::make_unique<WorldMapUpdate>(width, height, std::move(cells));
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_snapshot() {
    uint32_t tick = recv_u32();
    uint16_t n = recv_u16();
    std::vector<PlayerSnapshot> players;
    players.reserve(n);
    for (uint16_t i = 0; i < n; ++i) {
        PlayerSnapshot p;
        p.player_id = recv_u32();
        p.nick = recv_string();
        p.race = recv_u8();
        p.klass = recv_u8();
        p.x = recv_i32();
        p.y = recv_i32();
        p.hp = recv_i32();
        p.max_hp = recv_i32();
        p.mp = recv_i32();
        p.max_mp = recv_i32();
        p.xp = recv_u64();
        p.gold = recv_u64();
        p.level = recv_u16();
        p.is_ghost = (recv_u8() != 0);
        p.is_meditating = (recv_u8() != 0);
        players.push_back(std::move(p));
    }

    uint16_t items_count = recv_u16();
    std::vector<GroundItemSnapshot> ground_items;
    ground_items.reserve(items_count);

    for (uint16_t i = 0; i < items_count; ++i) {
        GroundItemSnapshot gi;
        gi.x = recv_i32();
        gi.y = recv_i32();
        gi.is_gold = (recv_u8() != 0);
        gi.quantity = recv_u64();
        gi.name = recv_string();
        ground_items.push_back(std::move(gi));
    }

    return std::make_unique<SnapshotUpdate>(tick, std::move(players), std::move(ground_items));
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_moved() {
    uint32_t pid = recv_u32();
    int32_t x = recv_i32();
    int32_t y = recv_i32();
    return std::make_unique<MovedUpdate>(pid, Position{x, y});
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_error() {
    uint8_t code = recv_u8();
    std::string detail = recv_string();
    return std::make_unique<ErrorUpdate>(0, code, std::move(detail));
}
