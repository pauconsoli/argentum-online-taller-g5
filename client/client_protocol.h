#ifndef CLIENT_PROTOCOL_H
#define CLIENT_PROTOCOL_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "common/direction.h"
#include "common/socket.h"
#include "common/updates/game_update.h"

class ClientProtocol {
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

    std::unique_ptr<GameUpdate> recv_login_ok();
    std::unique_ptr<GameUpdate> recv_match_list();
    std::unique_ptr<GameUpdate> recv_match_created();
    std::unique_ptr<GameUpdate> recv_match_joined();
    std::unique_ptr<GameUpdate> recv_player_joined();
    std::unique_ptr<GameUpdate> recv_player_left();
    std::unique_ptr<GameUpdate> recv_player_spawned();
    std::unique_ptr<GameUpdate> recv_snapshot();
    std::unique_ptr<GameUpdate> recv_moved();
    std::unique_ptr<GameUpdate> recv_attacked();
    std::unique_ptr<GameUpdate> recv_death();
    std::unique_ptr<GameUpdate> recv_inventory();
    std::unique_ptr<GameUpdate> recv_error();

 public:
    explicit ClientProtocol(Socket& socket);

    void send_login(const std::string& nick);
    void send_list_matches();
    void send_create_match(const std::string& match_name, uint8_t max_players);
    void send_join_match(uint32_t match_id);
    void send_select_race_class(uint8_t race, uint8_t klass);
    void send_move(Direction dir);
    void send_disconnect();
    void send_leave_match();

    std::unique_ptr<GameUpdate> receive_update();

    ClientProtocol(const ClientProtocol&) = delete;
    ClientProtocol& operator=(const ClientProtocol&) = delete;
};

#endif
