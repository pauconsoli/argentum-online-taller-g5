#include "server_protocol.h"

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

#include <arpa/inet.h>

#include "common/cheat_type.h"
#include "common/commands/attack_command.h"
#include "common/commands/chat_command.h"
#include "common/commands/cheat_command.h"
#include "common/commands/clan_command.h"
#include "common/commands/drop_item_command.h"
#include "common/commands/equip_item_command.h"
#include "common/commands/interact_npc_command.h"
#include "common/commands/meditate_command.h"
#include "common/commands/move_command.h"
#include "common/commands/pick_up_item_command.h"
#include "common/commands/resurrect_command.h"
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
#include "common/updates/meditate_update.h"
#include "common/updates/moved_update.h"
#include "common/updates/npc_interact_update.h"
#include "common/updates/player_joined_update.h"
#include "common/updates/player_left_update.h"
#include "common/updates/revive_update.h"
#include "common/updates/snapshot_update.h"
#include "common/updates/spawned_update.h"
#include "common/updates/system_msg_update.h"
#include "common/updates/world_map_update.h"
#include "server/game/player_class.h"
#include "server/game/player_race.h"

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


// funciones de casteo para raza/clase que son ENUM
static PlayerRace to_race(uint8_t v) {
    switch (v) {
        case 0:
            return PlayerRace::HUMAN;
        case 1:
            return PlayerRace::ELF;
        case 2:
            return PlayerRace::DWARF;
        case 3:
            return PlayerRace::GNOME;
        default:
            throw LibError(0, "raza inválida: %d", v);
    }
}

static PlayerClass to_class(uint8_t v) {
    switch (v) {
        case 0:
            return PlayerClass::MAGE;
        case 1:
            return PlayerClass::CLERIC;
        case 2:
            return PlayerClass::PALADIN;
        case 3:
            return PlayerClass::WARRIOR;
        default:
            throw LibError(0, "clase inválida: %d", v);
    }
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
    return {to_race(race), to_class(klass)};
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

std::unique_ptr<ClientCommand> ServerProtocol::recv_attack_payload(uint32_t player_id) {
    uint32_t target_id = recv_u32();
    return std::make_unique<AttackCommand>(player_id, target_id);
}

std::unique_ptr<ClientCommand> ServerProtocol::recv_meditate_payload(uint32_t player_id) {
    return std::make_unique<MeditateCommand>(player_id);
}

std::unique_ptr<ClientCommand> ServerProtocol::recv_resurrect_payload(uint32_t player_id) {
    return std::make_unique<ResurrectCommand>(player_id);
}

std::unique_ptr<ClientCommand> ServerProtocol::recv_pick_up_payload(uint32_t player_id) {
    return std::make_unique<PickUpItemCommand>(player_id);
}

std::unique_ptr<ClientCommand> ServerProtocol::recv_drop_item_payload(uint32_t player_id) {
    uint8_t slot = recv_u8();
    return std::make_unique<DropItemCommand>(player_id, static_cast<int>(slot));
}

std::unique_ptr<ClientCommand> ServerProtocol::recv_equip_item_payload(uint32_t player_id) {
    uint8_t slot = recv_u8();
    return std::make_unique<EquipItemCommand>(player_id, static_cast<int>(slot));
}

std::unique_ptr<ClientCommand> ServerProtocol::recv_interact_payload(uint32_t player_id) {
    uint32_t npc_id = recv_u32();
    uint8_t type_val = recv_u8();
    NPCInteraction type = static_cast<NPCInteraction>(type_val);
    std::string arg = recv_string();
    int32_t amount = recv_i32();
    return std::make_unique<InteractNPCCommand>(player_id, npc_id, type, arg, amount);
}

std::unique_ptr<ClientCommand> ServerProtocol::recv_clan_payload(uint32_t player_id) {
    uint8_t action_val = recv_u8();
    ClanAction action = static_cast<ClanAction>(action_val);
    std::string arg = recv_string();
    return std::make_unique<ClanCommand>(player_id, action, std::move(arg));
}

std::unique_ptr<ClientCommand> ServerProtocol::recv_chat_payload(uint32_t player_id,
                                                                 const std::string& nick) {
    std::string text = recv_string();
    return std::make_unique<ChatCommand>(player_id, nick, std::move(text));
}

std::unique_ptr<ClientCommand> ServerProtocol::recv_private_chat_payload(uint32_t player_id,
                                                                         const std::string& nick) {
    std::string target_nick = recv_string();
    std::string text = recv_string();
    return std::make_unique<ChatCommand>(player_id, nick, std::move(text), std::move(target_nick));
}

std::unique_ptr<ClientCommand> ServerProtocol::recv_cheat_payload(uint32_t player_id) {
    uint8_t type_raw = recv_u8();
    CheatType cheat_type = static_cast<CheatType>(type_raw);
    return std::make_unique<CheatCommand>(player_id, cheat_type);
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
        case UpdateType::PLAYER_SPAWNED:
            send_player_spawned(update);
            break;
        case UpdateType::WORLD_MAP:
            send_world_map(update);
            break;
        case UpdateType::SNAPSHOT:
            send_snapshot(update);
            break;
        case UpdateType::MOVED:
            send_moved(update);
            break;
        case UpdateType::ATTACKED:
            send_attacked(update);
            break;
        case UpdateType::DEATH:
            send_death(update);
            break;
        case UpdateType::REVIVE:
            send_revive(update);
            break;
        case UpdateType::MEDITATE:
            send_meditate(update);
            break;
        case UpdateType::INVENTORY:
            send_inventory(update);
            break;
        case UpdateType::CATALOG:
            send_catalog(update);
            break;
        case UpdateType::NPC_INTERACT:
            send_npc_interact(update);
            break;
        case UpdateType::CHAT_MSG:
            send_chat_msg(update);
            break;
        case UpdateType::SYSTEM_MSG:
            send_system_msg(update);
            break;
        case UpdateType::CLAN_RESULT:
            send_clan_result(update);
            break;
        case UpdateType::CLAN_REVIEW:
            send_clan_review(update);
            break;
        case UpdateType::ERROR:
            send_error(update);
            break;
        default:
            // Tolerante: si llega un tipo que aún no implementé (STATS, REVIVE,
            // CHAT_MSG, etc.), lo loggeo y lo descarto. Así no rompe la conexión del cliente — solo
            // se pierde ese mensaje en particular
            std::cerr << "[PROTOCOL] UpdateType no implementado (descartado): "
                      << static_cast<int>(update.get_type()) << "\n";
            break;
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
    put_string(buf, u.nick);
    put_u32(buf, u.clan_id);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ServerProtocol::send_player_left: client closed connection");
    }
}

void ServerProtocol::send_player_spawned(const GameUpdate& update) {
    const auto& u = static_cast<const SpawnedUpdate&>(update);
    std::vector<uint8_t> buf;
    put_u8(buf, ServerOpcode::PLAYER_SPAWNED);
    put_u32(buf, u.get_player_id());
    put_string(buf, u.get_nick());
    put_u8(buf, static_cast<uint8_t>(u.get_race()));
    put_u8(buf, static_cast<uint8_t>(u.get_class()));
    put_i32(buf, u.get_pos().x);
    put_i32(buf, u.get_pos().y);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ServerProtocol::send_player_spawned: client closed connection");
    }
}

void ServerProtocol::send_world_map(const GameUpdate& update) {
    const auto& u = static_cast<const WorldMapUpdate&>(update);
    if (static_cast<size_t>(u.width) * static_cast<size_t>(u.height) != u.cells.size()) {
        throw std::length_error("send_world_map: width*height != cells.size()");
    }
    std::vector<uint8_t> buf;
    put_u8(buf, ServerOpcode::WORLD_MAP);
    put_u16(buf, u.width);
    put_u16(buf, u.height);
    buf.reserve(buf.size() + u.cells.size() * 2);
    for (const auto& c : u.cells) {
        put_u8(buf, c.terrain_type);
        put_u8(buf, c.blocking ? 1 : 0);
    }
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ServerProtocol::send_world_map: client closed connection");
    }
}

void ServerProtocol::send_attacked(const GameUpdate& update) {
    const auto& u = static_cast<const AttackUpdate&>(update);
    const AttackResult& r = u.get_result();
    std::vector<uint8_t> buf;
    put_u8(buf, ServerOpcode::ATTACKED);
    put_u32(buf, r.attacker_id);
    put_u32(buf, r.target_id);
    put_i32(buf, r.damage);
    put_u8(buf, r.evaded ? 1 : 0);
    put_u8(buf, r.target_died ? 1 : 0);
    put_u8(buf, r.is_healing ? 1 : 0);
    put_i32(buf, r.heal_amount);
    put_u8(buf, static_cast<uint8_t>(r.type));
    put_string(buf, r.weapon_or_spell_name);
    put_u8(buf, static_cast<uint8_t>(r.status));
    put_u32(buf, r.target_clan_id);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ServerProtocol::send_attacked: client closed connection");
    }
}

void ServerProtocol::send_death(const GameUpdate& update) {
    const auto& u = static_cast<const DeathUpdate&>(update);
    std::vector<uint8_t> buf;
    put_u8(buf, ServerOpcode::DEATH);
    put_u32(buf, u.get_dead_id());
    put_u32(buf, u.get_killer_id());
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ServerProtocol::send_death: client closed connection");
    }
}

void ServerProtocol::send_meditate(const GameUpdate& update) {
    const auto& u = static_cast<const MeditateUpdate&>(update);
    std::vector<uint8_t> buf;
    put_u8(buf, ServerOpcode::MEDITATE);
    put_u8(buf, static_cast<uint8_t>(u.get_status()));
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ServerProtocol::send_meditate: client closed connection");
    }
}

void ServerProtocol::send_revive(const GameUpdate& update) {
    const auto& u = static_cast<const ReviveUpdate&>(update);
    std::vector<uint8_t> buf;
    put_u8(buf, ServerOpcode::REVIVE);
    put_u8(buf, static_cast<uint8_t>(u.get_status()));
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ServerProtocol::send_revive: client closed connection");
    }
}

void ServerProtocol::send_inventory(const GameUpdate& update) {
    const auto& u = static_cast<const InventoryUpdate&>(update);
    const auto& items = u.get_items();
    if (items.size() > UINT16_MAX) {
        throw std::length_error("send_inventory: demasiados items");
    }
    std::vector<uint8_t> buf;
    put_u8(buf, ServerOpcode::INVENTORY);
    put_u32(buf, u.get_target_player_id());
    put_u16(buf, static_cast<uint16_t>(items.size()));
    for (const auto& slot : items) {
        put_string(buf, slot.item_name);
        put_u32(buf, slot.quantity);
        put_u8(buf, slot.is_equipped ? 1 : 0);
    }
    put_u64(buf, u.get_gold());
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ServerProtocol::send_inventory: client closed connection");
    }
}

void ServerProtocol::send_catalog(const GameUpdate& update) {
    const auto& u = static_cast<const CatalogUpdate&>(update);
    const auto& catalog = u.get_catalog();
    if (catalog.size() > UINT16_MAX) {
        throw std::length_error("send_catalog: demasiados items en el catálogo");
    }
    std::vector<uint8_t> buf;
    put_u8(buf, ServerOpcode::CATALOG);
    put_u16(buf, static_cast<uint16_t>(catalog.size()));
    for (const auto& item_name : catalog) {
        put_string(buf, item_name);
    }
    put_u64(buf, u.get_gold_in_bank());
    put_u8(buf, u.is_vault() ? 1 : 0);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ServerProtocol::send_catalog: client closed connection");
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
        put_u8(buf, p.is_meditating ? 1 : 0);
        put_u32(buf, p.clan_id);

        put_u8(buf, static_cast<uint8_t>(p.equipment.size()));
        for (const auto& eq_name : p.equipment) {
            put_string(buf, eq_name);
        }
    }

    if (u.npcs.size() > UINT16_MAX) {
        throw std::length_error("send_snapshot: demasiados npcs");
    }
    put_u16(buf, static_cast<uint16_t>(u.npcs.size()));
    for (const auto& n : u.npcs) {
        put_u32(buf, n.npc_id);
        put_string(buf, n.name);
        put_i32(buf, n.x);
        put_i32(buf, n.y);
        put_i32(buf, n.hp);
        put_i32(buf, n.max_hp);
        put_u8(buf, n.is_hostile ? 1 : 0);
        put_u32(buf, n.npc_type);
        put_u8(buf, n.direction);
        put_u8(buf, n.is_moving ? 1 : 0);
    }

    if (u.ground_items.size() > UINT16_MAX) {
        throw std::length_error("send_snapshot: demasiados items en el suelo");
    }
    put_u16(buf, static_cast<uint16_t>(u.ground_items.size()));
    for (const auto& gi : u.ground_items) {
        put_i32(buf, gi.x);
        put_i32(buf, gi.y);
        put_u8(buf, gi.is_gold ? 1 : 0);
        put_u64(buf, gi.quantity);
        put_string(buf, gi.name);
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

void ServerProtocol::send_chat_msg(const GameUpdate& update) {
    const auto& u = static_cast<const ChatMsgUpdate&>(update);
    std::vector<uint8_t> buf;
    put_u8(buf, ServerOpcode::CHAT_MSG);
    put_u32(buf, u.sender_id);
    put_string(buf, u.sender_nick);
    put_string(buf, u.text);
    put_u8(buf, u.is_private ? 1 : 0);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ServerProtocol::send_chat_msg: client closed connection");
    }
}

void ServerProtocol::send_system_msg(const GameUpdate& update) {
    const auto& u = static_cast<const SystemMsgUpdate&>(update);
    std::vector<uint8_t> buf;
    put_u8(buf, ServerOpcode::SYSTEM_MSG);
    put_u32(buf, u.get_target_player_id());
    put_string(buf, u.get_text());
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ServerProtocol::send_system_msg: client closed connection");
    }
}

void ServerProtocol::send_npc_interact(const GameUpdate& update) {
    const auto& u = static_cast<const NpcInteractUpdate&>(update);
    const InteractResult& r = u.get_result();
    std::vector<uint8_t> buf;
    put_u8(buf, ServerOpcode::NPC_INTERACT);
    put_u8(buf, static_cast<uint8_t>(u.get_npc_interaction_type()));
    put_u8(buf, static_cast<uint8_t>(r.status));
    put_string(buf, r.item_name);
    put_u64(buf, r.gold_amount);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ServerProtocol::send_npc_interact: client closed connection");
    }
}

void ServerProtocol::send_clan_result(const GameUpdate& update) {
    const auto& u = static_cast<const ClanResultUpdate&>(update);
    const ClanResult& r = u.get_result();
    std::vector<uint8_t> buf;
    put_u8(buf, ServerOpcode::CLAN_RESULT);
    put_u8(buf, static_cast<uint8_t>(u.get_action()));
    put_u8(buf, static_cast<uint8_t>(r.status));
    put_string(buf, r.clan_name);
    put_string(buf, r.actor_nick);
    put_u32(buf, r.other_player_id);
    put_string(buf, r.other_nick);
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ServerProtocol::send_clan_result: client closed connection");
    }
}

void ServerProtocol::send_clan_review(const GameUpdate& update) {
    const auto& u = static_cast<const ClanReviewUpdate&>(update);
    std::vector<uint8_t> buf;
    put_u8(buf, ServerOpcode::CLAN_REVIEW);
    put_string(buf, u.get_clan_name());
    put_u16(buf, static_cast<uint16_t>(u.get_members().size()));
    for (const auto& m : u.get_members()) {
        put_u32(buf, m.player_id);
        put_string(buf, m.nick);
        put_u8(buf, m.is_founder ? 1 : 0);
        put_u8(buf, m.is_online ? 1 : 0);
    }
    put_u16(buf, static_cast<uint16_t>(u.get_pending().size()));
    for (const auto& p : u.get_pending()) {
        put_u32(buf, p.player_id);
        put_string(buf, p.nick);
        put_u8(buf, 0);
        put_u8(buf, p.is_online ? 1 : 0);
    }
    if (skt.sendall(buf.data(), buf.size()) == 0) {
        throw LibError(0, "%s", "ServerProtocol::send_clan_review: client closed connection");
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
