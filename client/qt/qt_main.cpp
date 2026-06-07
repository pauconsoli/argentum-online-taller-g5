// Entry point del cliente con menú Qt.
//
// Binario: taller_client_qt (ver CMakeLists raíz).
//
// Cuando esté integrado el handoff a SDL, este main:
//   - Conecta gameStartRequested del MainWindow a una función que
//     instancia el GameClient (SDL) con el Client que llega por move.
//   - Levanta SDL en el mismo proceso después de cerrar la ventana Qt.
//
// Por ahora muestra solo el menú: conexión → lobby → raza/clase.

#include <QApplication>
#include <QMetaType>
#include "common/updates/match_list_update.h"

#include "main_window.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    qRegisterMetaType<std::vector<MatchInfo>>("std::vector<MatchInfo>");
    qRegisterMetaType<uint8_t>("uint8_t");
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
    window.show();

    return app.exec();
}
