#include "client.h"
#include "common/updates/attack_update.h"
#include "common/updates/error_update.h"
#include "common/updates/match_created_update.h"
#include "common/updates/match_joined_update.h"
#include "common/updates/match_list_update.h"

#include <iostream>
#include <memory>
#include <utility>

#include <sys/socket.h>

#include "common/liberror.h"


namespace {

class ClientReceiverThread: public Thread {
 private:
    Client* client;
    ClientProtocol& protocol;
    Socket& socket;

 public:
    ClientReceiverThread(Client* c, ClientProtocol& p, Socket& s):
            client(c), protocol(p), socket(s) {}

    void run() override {
        try {
            while (should_keep_running()) {
                auto update = protocol.receive_update();

            if (!update) {
                continue;
            }

            switch (update->get_type()) {
                case UpdateType::LOGIN_OK: {
                    emit client->loginOk();
                    break;
                }

                case UpdateType::MATCH_LIST: {
                    auto* u = static_cast<MatchListUpdate*>(update.get());
                    emit client->matchListReceived(u->matches);
                    break;
                }

                case UpdateType::MATCH_CREATED: {
                    emit client->matchCreated();
                    break;
                }

                case UpdateType::MATCH_JOINED: {
                    emit client->matchJoined();
                    break;
                }

                case UpdateType::ERROR: {
                    auto* u = static_cast<ErrorUpdate*>(update.get());
                    emit client->errorReceived(
                            u->code,
                            QString::fromStdString(u->detail));
                    break;
                }

                case UpdateType::ATTACKED: {
                    auto* u = static_cast<AttackUpdate*>(update.get());
                    emit client->attackReceived(u->get_result());
                    break;
                }

                default:
                    break;
            }
        }
        } catch (const LibError& e) {
            std::cerr << "[CLIENT-RECEIVER] Conexión cerrada: " << e.what() << "\n";
        } catch (const std::exception& e) {
            std::cerr << "[CLIENT-RECEIVER] Excepción inesperada: " << e.what() << "\n";
        }

        emit client->disconnectedFromServer();
    }

    void stop() override {
        Thread::stop();
        try {
            socket.shutdown(SHUT_RD);
        } catch (...) {}
    }
};

}  // namespace

Client::Client(const std::string& host, const std::string& port, QObject* parent):
        QObject(parent),    
        socket(host.c_str(), port.c_str()),
        protocol(socket),
        send_mutex(),
        receiver_thread(nullptr) {}

Client::~Client() {
    try {
        stop();
        join();
    } catch (...) {}
}

void Client::start() {
    if (receiver_thread) {
        return;
    }
    receiver_thread =
             std::make_unique<ClientReceiverThread>(this, protocol, socket);
    receiver_thread->start();
}

void Client::stop() {
    if (!receiver_thread) {
        return;
    }
    receiver_thread->stop();
}

void Client::join() {
    if (!receiver_thread) {
        return;
    }
    if (receiver_thread->is_alive()) {
        receiver_thread->join();
    }
}

void Client::do_login(const std::string& nick) {
    std::lock_guard<std::mutex> lk(send_mutex);
    protocol.send_login(nick);
}

void Client::do_list_matches() {
    std::lock_guard<std::mutex> lk(send_mutex);
    protocol.send_list_matches();
}

void Client::do_create_match(const std::string& name, uint8_t max_players) {
    std::lock_guard<std::mutex> lk(send_mutex);
    protocol.send_create_match(name, max_players);
}

void Client::do_join_match(uint32_t match_id) {
    std::lock_guard<std::mutex> lk(send_mutex);
    protocol.send_join_match(match_id);
}

void Client::do_select_race_class(uint8_t race, uint8_t klass) {
    std::lock_guard<std::mutex> lk(send_mutex);
    protocol.send_select_race_class(race, klass);
}

void Client::do_move(Direction dir) {
    std::lock_guard<std::mutex> lk(send_mutex);
    protocol.send_move(dir);
}

void Client::do_attack(uint32_t target_id) {
    std::lock_guard<std::mutex> lk(send_mutex);
    protocol.send_attack(target_id);
}

void Client::do_meditate() {
    std::lock_guard<std::mutex> lk(send_mutex);
    protocol.send_meditate();
}

void Client::do_pick_up() {
    std::lock_guard<std::mutex> lk(send_mutex);
    protocol.send_pick_up();
}

void Client::do_drop_item(uint8_t slot_index) {
    std::lock_guard<std::mutex> lk(send_mutex);
    protocol.send_drop_item(slot_index);
}

void Client::do_leave_match() {
    std::lock_guard<std::mutex> lk(send_mutex);
    protocol.send_leave_match();
}

void Client::do_disconnect() {
    std::lock_guard<std::mutex> lk(send_mutex);
    protocol.send_disconnect();
}

