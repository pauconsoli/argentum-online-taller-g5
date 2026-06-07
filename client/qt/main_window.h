#ifndef QT_MAIN_WINDOW_H
#define QT_MAIN_WINDOW_H

#include <cstdint>
#include <memory>

#include <QMainWindow>
#include <QString>

#include "client/client.h"

class QStackedWidget;

class ConnectionWidget;
class LobbyWidget;
class RaceClassWidget;

class MainWindow: public QMainWindow {
    Q_OBJECT

 public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

 signals:
    //void gameStartRequested(std::unique_ptr<Client> client); => me rompe cuando compilo

 private slots:
    void handle_connect_requested(const QString& host, const QString& port,
                                  const QString& nick);

    void handle_refresh_requested();
    void handle_create_match_requested(const QString& name, uint8_t max_players);
    void handle_join_match_requested(uint32_t match_id);
    void handle_confirm_race_class(uint8_t race, uint8_t klass);


 private:
    enum Page { PAGE_CONNECTION = 0, PAGE_LOBBY = 1, PAGE_RACE_CLASS = 2 };

    QStackedWidget* stack;
    ConnectionWidget* connection_page;
    LobbyWidget* lobby_page;
    RaceClassWidget* race_class_page;

    std::unique_ptr<Client> client;

    void show_error_in_current_page(const QString& msg);
    void connect_client_signals();
};

#endif
