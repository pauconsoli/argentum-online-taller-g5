#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "common/commands/client_command.h"
#include "common/socket.h"
#include "common/updates/game_update.h"
#include "common/updates/match_list_update.h"
#include "common/updates/snapshot_update.h"
#include "server/game/player_class.h"
#include "server/game/player_race.h"

class ServerProtocol {
 private:
    Socket& skt;

    static void put_u8(std::vector<uint8_t>& buf, uint8_t v);
    static void put_u16(std::vector<uint8_t>& buf, uint16_t v);
    static void put_u32(std::vector<uint8_t>& buf, uint32_t v);
    static void put_i32(std::vector<uint8_t>& buf, int32_t v);
    static void put_u64(std::vector<uint8_t>& buf, uint64_t v);
    static void put_string(std::vector<uint8_t>& buf, const std::string& s);

    uint8_t recv_u8();
    uint16_t recv_u16();
    uint32_t recv_u32();
    int32_t recv_i32();
    uint64_t recv_u64();
    std::string recv_string();

    void send_login_ok(const GameUpdate& update);
    void send_match_list(const GameUpdate& update);
    void send_match_created(const GameUpdate& update);
    void send_match_joined(const GameUpdate& update);
    void send_player_joined(const GameUpdate& update);
    void send_player_left(const GameUpdate& update);
    void send_player_spawned(const GameUpdate& update);
    void send_world_map(const GameUpdate& update);
    void send_snapshot(const GameUpdate& update);
    void send_moved(const GameUpdate& update);
    void send_attacked(const GameUpdate& update);
    void send_death(const GameUpdate& update);
    void send_revive(const GameUpdate& update);
    void send_inventory(const GameUpdate& update);
    void send_chat_msg(const GameUpdate& update);
    void send_system_msg(const GameUpdate& update);
    void send_npc_interact(const GameUpdate& update);
    void send_clan_result(const GameUpdate& update);
    void send_clan_review(const GameUpdate& update);
    void send_error(const GameUpdate& update);
    void send_catalog(const GameUpdate& update);

 public:
    explicit ServerProtocol(Socket& socket);
    uint8_t recv_opcode();

    std::string recv_login_payload();
    struct CreateMatchPayload {
        std::string name;
        uint8_t max_players;
    };
    CreateMatchPayload recv_create_match_payload();
    uint32_t recv_join_match_payload();
    struct RaceClassPayload {
        PlayerRace race;
        PlayerClass klass;
    };
    RaceClassPayload recv_select_race_class_payload();

    std::unique_ptr<ClientCommand> recv_move_payload(uint32_t player_id);
    std::unique_ptr<ClientCommand> recv_attack_payload(uint32_t player_id);
    std::unique_ptr<ClientCommand> recv_meditate_payload(uint32_t player_id);
    std::unique_ptr<ClientCommand> recv_resurrect_payload(uint32_t player_id);
    std::unique_ptr<ClientCommand> recv_pick_up_payload(uint32_t player_id);
    std::unique_ptr<ClientCommand> recv_drop_item_payload(uint32_t player_id);
    std::unique_ptr<ClientCommand> recv_equip_item_payload(uint32_t player_id);
    std::unique_ptr<ClientCommand> recv_interact_payload(uint32_t player_id);
    std::unique_ptr<ClientCommand> recv_clan_payload(uint32_t player_id);
    std::unique_ptr<ClientCommand> recv_chat_payload(uint32_t player_id, const std::string& nick);
    std::unique_ptr<ClientCommand> recv_cheat_payload(uint32_t player_id);

    void send_update(const GameUpdate& update);

    ServerProtocol(const ServerProtocol&) = delete;
    ServerProtocol& operator=(const ServerProtocol&) = delete;
};

#endif
