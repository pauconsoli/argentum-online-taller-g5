#ifndef CLIENT_H
#define CLIENT_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <QObject>
#include <QString>
#include <QMetaType>

#include "client_protocol.h"
#include "common/direction.h"
#include "common/socket.h"
#include "common/thread.h"
#include "common/updates/match_list_update.h"

#include "client_protocol.h"

Q_DECLARE_METATYPE(std::vector<MatchInfo>)

class Client : public QObject {
    Q_OBJECT

 private:
    Socket socket;
    ClientProtocol protocol;

    std::mutex send_mutex;
    std::unique_ptr<Thread> receiver_thread;

 public:
    Client(const std::string& host, const std::string& port, QObject* parent = nullptr);

    void start();
    void stop();
    void join();

    void do_login(const std::string& nick);
    void do_list_matches();
    void do_create_match(const std::string& name, uint8_t max_players);
    void do_join_match(uint32_t match_id);
    void do_select_race_class(uint8_t race, uint8_t klass);
    void do_move(Direction dir);
    void do_leave_match();
    void do_disconnect();

    ~Client() override;

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

 signals:
    void loginOk();
    void matchListReceived(const std::vector<MatchInfo>& matches);
    void matchCreated();
    void matchJoined();
    void errorReceived(uint8_t code, QString detail);
    void disconnectedFromServer();
};

#endif