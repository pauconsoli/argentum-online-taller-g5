#ifndef QT_CLIENT_ADAPTER_H
#define QT_CLIENT_ADAPTER_H

#include <QMetaType>
#include <QObject>
#include <QString>
#include <QTimer>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "client/client.h"
#include "common/attack_result.h"
#include "common/updates/match_list_update.h"
#include "common/updates/world_map_update.h"

Q_DECLARE_METATYPE(std::vector<MatchInfo>)
Q_DECLARE_METATYPE(AttackResult)
Q_DECLARE_METATYPE(std::vector<MapCellData>)

class QtClientAdapter: public QObject {
    Q_OBJECT

 public:
    explicit QtClientAdapter(Client* client, QObject* parent = nullptr);
    ~QtClientAdapter() override;

    void start();

    void stop();

    QtClientAdapter(const QtClientAdapter&) = delete;
    QtClientAdapter& operator=(const QtClientAdapter&) = delete;

 signals:
    void loginOk(uint32_t player_id);
    void matchListReceived(const std::vector<MatchInfo>& matches);
    void matchCreated();
    void matchJoined();
    void errorReceived(uint8_t code, QString detail);
    void disconnectedFromServer();

    void attackReceived(AttackResult result);
    void worldMapReceived(uint16_t width, uint16_t height, std::vector<MapCellData> cells);
    void chatMessageReceived(uint32_t sender_id, QString sender_nick, QString text);

 private slots:
    void poll_updates();

 private:
    Client* client;
    QTimer* timer;
    static constexpr int POLL_MS = 50;
    static constexpr int UPDATES_PER_TICK = 16;
};

#endif
