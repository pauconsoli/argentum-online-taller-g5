#ifndef QT_MAIN_WINDOW_H
#define QT_MAIN_WINDOW_H

#include <QCloseEvent>
#include <QMainWindow>
#include <QString>
#include <cstdint>
#include <memory>

#include "client/client.h"

class QStackedWidget;
class QLabel;

class ConnectionWidget;
class LobbyWidget;
class RaceClassWidget;
class QtClientAdapter;

class MainWindow: public QMainWindow {
    Q_OBJECT

 public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

 signals:
    // Emitida cuando el usuario completó login+lobby+raza/clase.
    // Transfiere el Client ya conectado al loop SDL (mismo proceso).
    void gameStartRequested(Client* client, uint8_t race, uint8_t klass, uint32_t player_id);

 protected:
    void closeEvent(QCloseEvent* event) override;

 private slots:
    void handle_connect_requested(const QString& host, const QString& port, const QString& nick);

    void handle_refresh_requested();
    void handle_create_match_requested(const QString& name, uint8_t max_players);
    void handle_join_match_requested(uint32_t match_id);
    void handle_confirm_race_class(uint8_t race, uint8_t klass);

    void handle_logout_requested();
    void handle_back_to_lobby_requested();

 private:
    enum Page { PAGE_CONNECTION = 0, PAGE_LOBBY = 1, PAGE_RACE_CLASS = 2 };

    QStackedWidget* stack;
    ConnectionWidget* connection_page;
    LobbyWidget* lobby_page;
    RaceClassWidget* race_class_page;

    std::unique_ptr<Client> client;
    std::unique_ptr<QtClientAdapter> adapter;

    QString current_nick;
    uint32_t current_player_id;
    QLabel* status_user_label;

    void show_error_in_current_page(const QString& msg);
    void connect_adapter_signals();
    void update_status_bar();
    void teardown_session();
};

#endif
