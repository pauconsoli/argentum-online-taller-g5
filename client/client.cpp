#include "client.h"

#include <iostream>
#include <memory>
#include <utility>

#include <sys/socket.h>

#include "common/liberror.h"


namespace {

class ClientReceiverThread: public Thread {
 private:
    ClientProtocol& protocol;
    Queue<std::unique_ptr<GameUpdate>>& out_queue;
    Socket& socket;

 public:
    ClientReceiverThread(ClientProtocol& p, Queue<std::unique_ptr<GameUpdate>>& q, Socket& s):
            protocol(p), out_queue(q), socket(s) {}

    void run() override {
        try {
            while (should_keep_running()) {
                auto update = protocol.receive_update();
                if (update) {
                    out_queue.push(std::move(update));
                }
            }
        } catch (const ClosedQueue&) {
            // salgo
        } catch (const LibError& e) {
            std::cerr << "[CLIENT-RECEIVER] Conexión cerrada: " << e.what() << "\n";
        } catch (const std::exception& e) {
            std::cerr << "[CLIENT-RECEIVER] Excepción inesperada: " << e.what() << "\n";
        }

        try {
            out_queue.close();
        } catch (...) {}
    }

    void stop() override {
        Thread::stop();
        try {
            socket.shutdown(SHUT_RD);
        } catch (...) {}
    }
};

}

Client::Client(const std::string& host, const std::string& port):
        socket(host.c_str(), port.c_str()),
        protocol(socket),
        send_mutex(),
        received_updates(),
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
            std::make_unique<ClientReceiverThread>(protocol, received_updates, socket);
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

void Client::do_leave_match() {
    std::lock_guard<std::mutex> lk(send_mutex);
    protocol.send_leave_match();
}

void Client::do_disconnect() {
    std::lock_guard<std::mutex> lk(send_mutex);
    protocol.send_disconnect();
}

Queue<std::unique_ptr<GameUpdate>>& Client::get_received_updates() {
    return received_updates;
}
