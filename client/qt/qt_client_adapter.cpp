#include "qt_client_adapter.h"

#include "common/queue.h"
#include "common/updates/attack_update.h"
#include "common/updates/error_update.h"
#include "common/updates/login_ok_update.h"
#include "common/updates/match_created_update.h"
#include "common/updates/match_joined_update.h"
#include "common/updates/match_list_update.h"
#include "common/updates/world_map_update.h"

QtClientAdapter::QtClientAdapter(Client* client_, QObject* parent):
    QObject(parent), client(client_), timer(new QTimer(this)) {
    connect(timer, &QTimer::timeout, this, &QtClientAdapter::poll_updates);
}

QtClientAdapter::~QtClientAdapter() = default;

void QtClientAdapter::start() {
    if (!client)
        return;
    client->start();
    timer->start(POLL_MS);
}

void QtClientAdapter::stop() {
    if (timer->isActive()) {
        timer->stop();
    }
}

void QtClientAdapter::poll_updates() {
    if (!client)
        return;
    auto& q = client->get_received_updates();
    std::unique_ptr<GameUpdate> u;

    for (int i = 0; i < UPDATES_PER_TICK; ++i) {
        try {
            if (!q.try_pop(u)) {
                break;
            }
        } catch (const ClosedQueue&) {
            timer->stop();
            emit disconnectedFromServer();
            return;
        }

        if (!u) {
            continue;
        }

        switch (u->get_type()) {
            case UpdateType::LOGIN_OK: {
                const auto* x = static_cast<const LoginOkUpdate*>(u.get());
                emit loginOk(x->player_id);
                break;
            }
            case UpdateType::MATCH_LIST: {
                const auto* x = static_cast<const MatchListUpdate*>(u.get());
                emit matchListReceived(x->matches);
                break;
            }
            case UpdateType::MATCH_CREATED:
                emit matchCreated();
                break;
            case UpdateType::MATCH_JOINED:
                timer->stop();  // leave WORLD_MAP and snapshots in queue for SDL
                emit matchJoined();
                return;
            case UpdateType::ERROR: {
                const auto* x = static_cast<const ErrorUpdate*>(u.get());
                emit errorReceived(x->code, QString::fromStdString(x->detail));
                break;
            }
            case UpdateType::ATTACKED: {
                auto* x = static_cast<AttackUpdate*>(u.get());
                emit attackReceived(x->get_result());
                break;
            }
            case UpdateType::WORLD_MAP: {
                const auto* x = static_cast<const WorldMapUpdate*>(u.get());
                emit worldMapReceived(x->width, x->height, x->cells);
                break;
            }
            default:
                // Updates in-game (MOVED, SNAPSHOT ..) Chiari
                break;
        }
    }
}
