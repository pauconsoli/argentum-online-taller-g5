#ifndef CLIENT_H
#define CLIENT_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

#include "client_protocol.h"
#include "common/cheat_type.h"
#include "common/clan/clan_action.h"
#include "common/direction.h"
#include "common/npc_interaction.h"
#include "common/queue.h"
#include "common/socket.h"
#include "common/thread.h"

class Client {
 private:
    Socket socket;
    ClientProtocol protocol;

    std::mutex send_mutex;

    Queue<std::unique_ptr<GameUpdate>> received_updates;

    std::unique_ptr<Thread> receiver_thread;

 public:
    Client(const std::string& host, const std::string& port);

    void start();
    void stop();
    void join();

    void do_login(const std::string& nick);
    void do_list_matches();
    void do_create_match(const std::string& name, uint8_t max_players);
    void do_join_match(uint32_t match_id);
    void do_select_race_class(uint8_t race, uint8_t klass);
    void do_move(Direction dir);
    void do_attack(uint32_t target_id);
    void do_meditate();
    void do_resurrect();
    void do_pick_up();
    void do_drop_item(uint8_t slot_index);
    void do_equip_item(uint8_t slot_index);
    void do_chat(const std::string& text);
    void do_private_chat(const std::string& target_nick, const std::string& text);
    void do_interact(uint32_t npc_id, NPCInteraction type, const std::string& arg, int32_t amount);
    void do_clan_action(ClanAction action, const std::string& arg);
    void do_cheat(CheatType cheat_type);
    void do_leave_match();
    void do_disconnect();

    Queue<std::unique_ptr<GameUpdate>>& get_received_updates();

    ~Client();

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
};

#endif
