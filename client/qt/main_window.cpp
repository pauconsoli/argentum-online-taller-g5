#include "main_window.h"

#include <stdexcept>
#include <utility>

#include <QMessageBox>
#include <QStackedWidget>
#include <QString>


#include "connection_widget.h"
#include "lobby_widget.h"
#include "race_class_widget.h"

MainWindow::MainWindow(QWidget* parent):
        QMainWindow(parent),
        stack(new QStackedWidget(this)),
        connection_page(new ConnectionWidget(this)),
        lobby_page(new LobbyWidget(this)),
        race_class_page(new RaceClassWidget(this)),
        client(nullptr){

    setWindowTitle(tr("Argentum Online - G5"));
    resize(640, 480);

    stack->addWidget(connection_page);  // 0
    stack->addWidget(lobby_page);       // 1
    stack->addWidget(race_class_page);  // 2
    setCentralWidget(stack);

    stack->setCurrentIndex(PAGE_CONNECTION);

    connect(connection_page, &ConnectionWidget::connectRequested, this,
            &MainWindow::handle_connect_requested);

    connect(lobby_page, &LobbyWidget::refreshRequested, this,
            &MainWindow::handle_refresh_requested);
    connect(lobby_page, &LobbyWidget::createMatchRequested, this,
            &MainWindow::handle_create_match_requested);
    connect(lobby_page, &LobbyWidget::joinMatchRequested, this,
            &MainWindow::handle_join_match_requested);

    connect(race_class_page, &RaceClassWidget::confirmRequested, this,
            &MainWindow::handle_confirm_race_class);

}

MainWindow::~MainWindow() = default;

void MainWindow::handle_connect_requested(const QString& host, const QString& port,
                                          const QString& nick) {
    connection_page->set_busy(true);
    try {
        client = std::make_unique<Client>(host.toStdString(), port.toStdString());
        connect_client_signals();
        client->start();
        client->do_login(nick.toStdString());
    } catch (const std::exception& e) {
        connection_page->show_error(tr("No se pudo conectar: %1").arg(QString::fromLatin1(e.what())));
        connection_page->set_busy(false);
        client.reset();
    }
}

void MainWindow::handle_refresh_requested() {
    if (!client) return;
    try {
        client->do_list_matches();
    } catch (const std::exception& e) {
        lobby_page->show_error(tr("Error al pedir lista: %1").arg(QString::fromLatin1(e.what())));
    }
}

void MainWindow::handle_create_match_requested(const QString& name, uint8_t max_players) {
    if (!client) return;
    try {
        client->do_create_match(name.toStdString(), max_players);
    } catch (const std::exception& e) {
        lobby_page->show_error(tr("Error al crear: %1").arg(QString::fromLatin1(e.what())));
    }
}

void MainWindow::handle_join_match_requested(uint32_t match_id) {
    if (!client) return;
    try {
        client->do_join_match(match_id);
    } catch (const std::exception& e) {
        lobby_page->show_error(tr("Error al unirse: %1").arg(QString::fromLatin1(e.what())));
    }
}

void MainWindow::handle_confirm_race_class(uint8_t race, uint8_t klass) {
    if (!client) return;
    try {
        client->do_select_race_class(race, klass);
    } catch (const std::exception& e) {
        QMessageBox::warning(this, tr("Error"),
                             tr("No se pudo enviar la selección: %1")
                                     .arg(QString::fromLatin1(e.what())));
        return;
    }

    // HANDOFF A SDL (pendiente de Chiari):
    //   emit gameStartRequested(std::move(client));
    //   close();
    QMessageBox::information(this, tr("Listo"),
                             tr("Raza/Clase enviadas. El handoff a SDL todavía "
                                "no está integrado (Chiari)."));
}

void MainWindow::connect_client_signals() {
    connect(client.get(), &Client::loginOk, this, [this]() {
        stack->setCurrentIndex(PAGE_LOBBY);
        connection_page->set_busy(false);
        handle_refresh_requested();
    });

    connect(client.get(), &Client::matchListReceived,
            lobby_page, &LobbyWidget::set_matches);

    connect(client.get(), &Client::matchCreated, this, [this]() {
        handle_refresh_requested();
    });

    connect(client.get(), &Client::matchJoined, this, [this]() {
        stack->setCurrentIndex(PAGE_RACE_CLASS);
    });

    connect(client.get(), &Client::errorReceived, this,
            [this](uint8_t code, const QString& detail) {
                show_error_in_current_page(
                        tr("[err %1] %2")
                                .arg(static_cast<int>(code))
                                .arg(detail));
                connection_page->set_busy(false);
            });

    connect(client.get(), &Client::disconnectedFromServer, this, [this]() {
        if (!client) return;

        QMessageBox::warning(this, tr("Desconectado"),
                             tr("Se cerró la conexión con el servidor."));

        client.reset();
        connection_page->set_busy(false);
        stack->setCurrentIndex(PAGE_CONNECTION);
    });
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
