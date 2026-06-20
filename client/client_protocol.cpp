#include "client_protocol.h"

#include <cstring>
#include <stdexcept>
#include <utility>

#include <arpa/inet.h>

#include "common/attack_result.h"
#include "common/clan/clan_action.h"
#include "common/clan/clan_result.h"
#include "common/clan/clan_review_result.h"
#include "common/liberror.h"
#include "common/protocol_constants.h"
#include "common/updates/attack_update.h"
#include "common/updates/catalog_update.h"
#include "common/updates/chat_msg_update.h"
#include "common/updates/clan_result_update.h"
#include "common/updates/clan_review_update.h"
#include "common/updates/death_update.h"
#include "common/updates/error_update.h"
#include "common/updates/inventory_update.h"
#include "common/updates/login_ok_update.h"
#include "common/updates/match_created_update.h"
#include "common/updates/match_joined_update.h"
#include "common/updates/match_list_update.h"
#include "common/updates/moved_update.h"
#include "common/updates/npc_interact_update.h"
#include "common/updates/player_joined_update.h"
#include "common/updates/player_left_update.h"
#include "common/updates/snapshot_update.h"
#include "common/updates/spawned_update.h"
#include "common/updates/system_msg_update.h"
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

void ClientProtocol::send_equip_item(uint8_t slot_index) {
    std::vector<uint8_t> buf;
    put_u8(buf, ClientOpcode::EQUIP_ITEM);
    put_u8(buf, slot_index);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ClientProtocol::send_equip_item: server closed connection");
    }
}

void ClientProtocol::send_chat(const std::string& text) {
    std::vector<uint8_t> buf;
    put_u8(buf, ClientOpcode::CHAT);
    put_string(buf, text);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ClientProtocol::send_chat: server closed connection");
    }
}

void ClientProtocol::send_interact(uint32_t npc_id, NPCInteraction type, const std::string& arg,
                                   int32_t amount) {
    std::vector<uint8_t> buf;
    put_u8(buf, ClientOpcode::INTERACT);
    put_u32(buf, npc_id);
    put_u8(buf, static_cast<uint8_t>(type));
    put_string(buf, arg);
    put_i32(buf, amount);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ClientProtocol::send_interact: server closed connection");
    }
}

void ClientProtocol::send_clan_action(ClanAction action, const std::string& arg) {
    std::vector<uint8_t> buf;
    put_u8(buf, ClientOpcode::CLAN);
    put_u8(buf, static_cast<uint8_t>(action));
    put_string(buf, arg);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ClientProtocol::send_clan_action: server closed connection");
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
        case ServerOpcode::CHAT_MSG:
            return recv_chat_msg();
        case ServerOpcode::SYSTEM_MSG:
            return recv_system_msg();
        case ServerOpcode::NPC_INTERACT:
            return recv_npc_interact();
        case ServerOpcode::CLAN_RESULT:
            return recv_clan_result();
        case ServerOpcode::CLAN_REVIEW:
            return recv_clan_review();
        case ServerOpcode::ERROR:
            return recv_error();
        case ServerOpcode::CATALOG:
            return recv_catalog();
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
    return std::make_unique<PlayerLeftUpdate>(pid, "", 0);
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_attacked() {
    AttackResult r;
    r.attacker_id = recv_u32();
    r.target_id = recv_u32();
    r.damage = recv_i32();
    r.evaded = (recv_u8() != 0);
    r.target_died = (recv_u8() != 0);
    r.is_healing = (recv_u8() != 0);
    r.heal_amount = recv_i32();
    r.type = static_cast<AttackType>(recv_u8());
    r.weapon_or_spell_name = recv_string();
    return std::make_unique<AttackUpdate>(r);
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_death() {
    uint32_t dead_id = recv_u32();
    uint32_t killer_id = recv_u32();
    return std::make_unique<DeathUpdate>(dead_id, killer_id);
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_inventory() {
    uint32_t player_id = recv_u32();
    uint16_t n = recv_u16();
    std::vector<InventorySlotData> items;
    items.reserve(n);
    for (uint16_t i = 0; i < n; ++i) {
        InventorySlotData slot;
        slot.item_name = recv_string();
        slot.quantity = recv_u32();
        slot.is_equipped = recv_u8() != 0;
        items.push_back(std::move(slot));
    }
    uint64_t gold = recv_u64();
    return std::make_unique<InventoryUpdate>(player_id, std::move(items), gold);
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_player_spawned() {
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
        p.clan_id = recv_u32();

        uint8_t eq_count = recv_u8();
        p.equipment.reserve(eq_count);
        for (uint8_t j = 0; j < eq_count; ++j) {
            p.equipment.push_back(recv_string());
        }
        players.push_back(std::move(p));
    }

    uint16_t npcs_count = recv_u16();
    std::vector<NPCSnapshot> npcs;
    npcs.reserve(npcs_count);
    for (uint16_t i = 0; i < npcs_count; ++i) {
        NPCSnapshot npc_snapshot;
        npc_snapshot.npc_id = recv_u32();
        npc_snapshot.name = recv_string();
        npc_snapshot.x = recv_i32();
        npc_snapshot.y = recv_i32();
        npc_snapshot.hp = recv_i32();
        npc_snapshot.max_hp = recv_i32();
        npc_snapshot.is_hostile = (recv_u8() != 0);
        npc_snapshot.npc_type = recv_u32();
        npc_snapshot.direction = recv_u8();
        npc_snapshot.is_moving = (recv_u8() != 0);
        npcs.push_back(std::move(npc_snapshot));
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

    return std::make_unique<SnapshotUpdate>(tick, std::move(players), std::move(npcs),
                                            std::move(ground_items));
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_moved() {
    uint32_t pid = recv_u32();
    int32_t x = recv_i32();
    int32_t y = recv_i32();
    return std::make_unique<MovedUpdate>(pid, Position{x, y});
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_chat_msg() {
    uint32_t sender_id = recv_u32();
    std::string sender_nick = recv_string();
    std::string text = recv_string();
    return std::make_unique<ChatMsgUpdate>(sender_id, std::move(sender_nick), std::move(text));
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_system_msg() {
    uint32_t target_player_id = recv_u32();
    std::string text = recv_string();
    return std::make_unique<SystemMsgUpdate>(target_player_id, std::move(text));
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_npc_interact() {
    NPCInteraction type = static_cast<NPCInteraction>(recv_u8());
    InteractStatus status = static_cast<InteractStatus>(recv_u8());
    std::string item_name = recv_string();
    uint64_t gold_amount = recv_u64();
    InteractResult result(status, std::move(item_name), gold_amount);
    return std::make_unique<NpcInteractUpdate>(0, type, std::move(result));
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_clan_result() {
    uint8_t action_val = recv_u8();
    uint8_t status_val = recv_u8();
    std::string clan_name = recv_string();
    std::string actor_nick = recv_string();
    uint32_t other_player_id = recv_u32();
    std::string other_nick = recv_string();
    ClanAction action = static_cast<ClanAction>(action_val);
    ClanResult result;
    result.status = static_cast<ClanActionStatus>(status_val);
    result.clan_name = std::move(clan_name);
    result.actor_nick = std::move(actor_nick);
    result.other_player_id = other_player_id;
    result.other_nick = std::move(other_nick);
    return std::make_unique<ClanResultUpdate>(0, action, result);
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_clan_review() {
    std::string clan_name = recv_string();
    uint16_t member_count = recv_u16();
    std::vector<ClanMemberInfo> members;
    members.reserve(member_count);
    for (uint16_t i = 0; i < member_count; ++i) {
        uint32_t pid = recv_u32();
        std::string nick = recv_string();
        bool is_founder = recv_u8() != 0;
        bool is_online = recv_u8() != 0;
        members.push_back({pid, std::move(nick), is_founder, is_online});
    }
    uint16_t pending_count = recv_u16();
    std::vector<ClanMemberInfo> pending;
    pending.reserve(pending_count);
    for (uint16_t i = 0; i < pending_count; ++i) {
        uint32_t pid = recv_u32();
        std::string nick = recv_string();
        bool is_founder = recv_u8() != 0;
        bool is_online = recv_u8() != 0;
        pending.push_back({pid, std::move(nick), is_founder, is_online});
    }
    return std::make_unique<ClanReviewUpdate>(0, std::move(clan_name), std::move(members),
                                              std::move(pending));
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_error() {
    uint8_t code = recv_u8();
    std::string detail = recv_string();
    return std::make_unique<ErrorUpdate>(0, code, std::move(detail));
}

std::unique_ptr<GameUpdate> ClientProtocol::recv_catalog() {
    uint16_t n = recv_u16();
    std::vector<std::string> catalog;
    catalog.reserve(n);
    for (uint16_t i = 0; i < n; ++i) {
        catalog.push_back(recv_string());
    }
    uint64_t gold = recv_u64();
    return std::make_unique<CatalogUpdate>(0, std::move(catalog), gold);
}
