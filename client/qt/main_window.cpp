#include "main_window.h"

#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QStackedWidget>
#include <QStatusBar>
#include <QString>
#include <stdexcept>
#include <utility>

#include "connection_widget.h"
#include "lobby_widget.h"
#include "qt_client_adapter.h"
#include "race_class_widget.h"

MainWindow::MainWindow(QWidget* parent):
    QMainWindow(parent),
    stack(new QStackedWidget(this)),
    connection_page(new ConnectionWidget(this)),
    lobby_page(new LobbyWidget(this)),
    race_class_page(new RaceClassWidget(this)),
    client(nullptr),
    adapter(nullptr),
    current_nick(),
    current_player_id(0),
    status_user_label(new QLabel(this)) {

    setWindowTitle(tr("Argentum Online - G5"));
    setWindowIcon(QIcon(":/logo.png"));
    resize(720, 560);

    setStyleSheet(
        "QMainWindow {"
        "  background-image: url(:/background.jpg);"
        "  background-repeat: no-repeat;"
        "  background-position: center;"
        "  background-color: #1a1410;"
        "}"
        "QStackedWidget > QWidget {"
        "  background-color: rgba(20, 16, 12, 180);"
        "}");

    stack->addWidget(connection_page);  // 0
    stack->addWidget(lobby_page);       // 1
    stack->addWidget(race_class_page);  // 2
    setCentralWidget(stack);

    stack->setCurrentIndex(PAGE_CONNECTION);

    statusBar()->addWidget(status_user_label, 1);
    update_status_bar();

    connect(connection_page, &ConnectionWidget::connectRequested, this,
            &MainWindow::handle_connect_requested);

    connect(lobby_page, &LobbyWidget::refreshRequested, this,
            &MainWindow::handle_refresh_requested);
    connect(lobby_page, &LobbyWidget::createMatchRequested, this,
            &MainWindow::handle_create_match_requested);
    connect(lobby_page, &LobbyWidget::joinMatchRequested, this,
            &MainWindow::handle_join_match_requested);
    connect(lobby_page, &LobbyWidget::logoutRequested, this, &MainWindow::handle_logout_requested);

    connect(race_class_page, &RaceClassWidget::confirmRequested, this,
            &MainWindow::handle_confirm_race_class);
    connect(race_class_page, &RaceClassWidget::backToLobbyRequested, this,
            &MainWindow::handle_back_to_lobby_requested);
}

MainWindow::~MainWindow() = default;

void MainWindow::closeEvent(QCloseEvent* event) {
    if (client) {
        auto reply = QMessageBox::question(this, tr("¿Salir del juego?"),
                                           tr("Hay una sesión activa. ¿Querés cerrar el juego?"),
                                           QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (reply != QMessageBox::Yes) {
            event->ignore();
            return;
        }
    }
    event->accept();
}

void MainWindow::handle_connect_requested(const QString& host, const QString& port,
                                          const QString& nick) {
    connection_page->set_busy(true);
    try {
        client = std::make_unique<Client>(host.toStdString(), port.toStdString());

        adapter = std::make_unique<QtClientAdapter>(client.get(), this);
        connect_adapter_signals();

        adapter->start();

        client->do_login(nick.toStdString());
        current_nick = nick;
    } catch (const std::exception& e) {
        connection_page->show_error(
            tr("No se pudo conectar: %1").arg(QString::fromLatin1(e.what())));
        connection_page->set_busy(false);
        adapter.reset();
        client.reset();
    }
}

void MainWindow::handle_refresh_requested() {
    if (!client)
        return;
    try {
        client->do_list_matches();
    } catch (const std::exception& e) {
        lobby_page->show_error(tr("Error al pedir lista: %1").arg(QString::fromLatin1(e.what())));
    }
}

void MainWindow::handle_create_match_requested(const QString& name, uint8_t max_players) {
    if (!client)
        return;
    try {
        client->do_create_match(name.toStdString(), max_players);
    } catch (const std::exception& e) {
        lobby_page->show_error(tr("Error al crear: %1").arg(QString::fromLatin1(e.what())));
    }
}

void MainWindow::handle_join_match_requested(uint32_t match_id) {
    if (!client)
        return;
    try {
        client->do_join_match(match_id);
    } catch (const std::exception& e) {
        lobby_page->show_error(tr("Error al unirse: %1").arg(QString::fromLatin1(e.what())));
    }
}

void MainWindow::handle_confirm_race_class(uint8_t race, uint8_t klass) {
    if (!client)
        return;
    try {
        client->do_select_race_class(race, klass);
    } catch (const std::exception& e) {
        QMessageBox::warning(
            this, tr("Error"),
            tr("No se pudo enviar la selección: %1").arg(QString::fromLatin1(e.what())));
        return;
    }

    adapter->stop();
    adapter.reset();
    emit gameStartRequested(client.release(), race, klass, current_player_id);
    close();
}

void MainWindow::handle_logout_requested() {
    if (!client)
        return;
    auto reply = QMessageBox::question(this, tr("¿Cerrar sesión?"),
                                       tr("Vas a desconectarte del servidor. ¿Continuar?"),
                                       QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        return;
    }
    teardown_session();
}

void MainWindow::handle_back_to_lobby_requested() {
    if (!client)
        return;
    try {
        client->do_leave_match();
    } catch (const std::exception& e) {
        QMessageBox::warning(
            this, tr("Error"),
            tr("No se pudo salir del match: %1").arg(QString::fromLatin1(e.what())));
        return;
    }
    stack->setCurrentIndex(PAGE_LOBBY);
    update_status_bar();
    handle_refresh_requested();
}

void MainWindow::connect_adapter_signals() {
    connect(adapter.get(), &QtClientAdapter::loginOk, this, [this](uint32_t player_id) {
        current_player_id = player_id;
        stack->setCurrentIndex(PAGE_LOBBY);
        connection_page->set_busy(false);
        update_status_bar();
        handle_refresh_requested();
    });

    connect(adapter.get(), &QtClientAdapter::matchListReceived, lobby_page,
            &LobbyWidget::set_matches);

    connect(adapter.get(), &QtClientAdapter::matchCreated, this,
            [this]() { handle_refresh_requested(); });

    connect(adapter.get(), &QtClientAdapter::matchJoined, this, [this]() {
        stack->setCurrentIndex(PAGE_RACE_CLASS);
        update_status_bar();
    });

    connect(
        adapter.get(), &QtClientAdapter::errorReceived, this,
        [this](uint8_t code, const QString& detail) {
            show_error_in_current_page(tr("[err %1] %2").arg(static_cast<int>(code)).arg(detail));
            connection_page->set_busy(false);
        });

    connect(adapter.get(), &QtClientAdapter::disconnectedFromServer, this, [this]() {
        if (!client)
            return;
        QMessageBox::warning(this, tr("Desconectado"), tr("Se cerró la conexión con el servidor."));
        teardown_session();
        close();
    });
}

void MainWindow::teardown_session() {
    if (adapter) {
        adapter->stop();
    }
    adapter.reset();
    client.reset();
    current_nick.clear();
    connection_page->set_busy(false);
    stack->setCurrentIndex(PAGE_CONNECTION);
    update_status_bar();
}

void MainWindow::update_status_bar() {
    if (!client) {
        status_user_label->setText(tr("Desconectado"));
        return;
    }
    switch (stack->currentIndex()) {
        case PAGE_LOBBY:
            status_user_label->setText(tr("Conectado como  %1  ·  En lobby").arg(current_nick));
            break;
        case PAGE_RACE_CLASS:
            status_user_label->setText(
                tr("Conectado como  %1  ·  Eligiendo personaje").arg(current_nick));
            break;
        default:
            status_user_label->setText(tr("Conectado como  %1").arg(current_nick));
    }
}

void MainWindow::show_error_in_current_page(const QString& msg) {
    switch (stack->currentIndex()) {
        case PAGE_CONNECTION:
            connection_page->show_error(msg);
            break;
        case PAGE_LOBBY:
            lobby_page->show_error(msg);
            break;
        default:
            QMessageBox::warning(this, tr("Error"), msg);
            break;
    }
}
