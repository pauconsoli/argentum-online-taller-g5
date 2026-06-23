#include <QApplication>
#include <QMetaType>
#include <memory>

#include "client/client.h"
#include "client/sdl/core/sdl_config.h"
#include "client/sdl/game_client.h"
#include "common/attack_result.h"
#include "common/updates/match_list_update.h"
#include "common/updates/world_map_update.h"
#include "main_window.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    qRegisterMetaType<std::vector<MatchInfo>>("std::vector<MatchInfo>");
    qRegisterMetaType<uint8_t>("uint8_t");
    qRegisterMetaType<AttackResult>("AttackResult");
    qRegisterMetaType<std::vector<MapCellData>>("std::vector<MapCellData>");
    qRegisterMetaType<uint16_t>("uint16_t");
    app.setStyleSheet(R"(
        QMainWindow {
            background-color: #2b2118;
        }

        QWidget {
            background-color: #2b2118;
            color: #f5e6c8;
            font-family: "Georgia";
            font-size: 16px;
        }

        QPushButton {
            background-color: #6b3f1d;
            color: #f5e6c8;
            border: 2px solid #c89b3c;
            border-radius: 6px;
            padding: 8px;
            font-weight: bold;
        }

        QPushButton:hover {
            background-color: #8a5528;
        }

        QLineEdit, QSpinBox {
            background-color: #1f1712;
            color: #f5e6c8;
            border: 2px solid #8a6a2f;
            padding: 5px;
        }

        QTableWidget {
            background-color: #1b1510;
            color: #f5e6c8;
            gridline-color: #8a6a2f;
            border: 2px solid #8a6a2f;
        }

        QHeaderView::section {
            background-color: #4a2f1b;
            color: #f5e6c8;
            border: 1px solid #8a6a2f;
            font-weight: bold;
        }

        QLabel {
            color: #f5e6c8;
            font-weight: bold;
        }

        QStatusBar {
            background-color: #1b1510;
            color: #c89b3c;
            border-top: 1px solid #8a6a2f;
        }

        QStatusBar QLabel {
            color: #c89b3c;
            font-weight: normal;
            padding: 4px;
        }

        QGroupBox {
            border: 2px solid #8a6a2f;
            border-radius: 4px;
            margin-top: 12px;
            padding-top: 10px;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 6px;
            color: #c89b3c;
        }
    )");
    QCoreApplication::setApplicationName("Argentum Online - G5");

    MainWindow window;

    std::unique_ptr<Client> pending_client;
    uint8_t pending_race = 0;
    uint8_t pending_klass = 0;
    uint32_t pending_player_id = 0;

    QObject::connect(&window, &MainWindow::gameStartRequested,
                     [&pending_client, &pending_race, &pending_klass, &pending_player_id](
                         Client* c, uint8_t race, uint8_t klass, uint32_t player_id) {
                         pending_client.reset(c);
                         pending_race = race;
                         pending_klass = klass;
                         pending_player_id = player_id;
                     });

    window.show();
    app.exec();

    if (pending_client) {
        SdlConfig config = SdlConfig::load("client/config/sdl_config.toml");
        GameClient game(config.window_width, config.window_height, config.fullscreen,
                        std::move(pending_client), pending_race, pending_klass, pending_player_id);
        game.run();
    }

    return 0;
}
