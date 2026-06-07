#ifndef LOBBY_WIDGET_H
#define LOBBY_WIDGET_H

#include <cstdint>

#include <QString>
#include <QWidget>

#include "common/updates/match_list_update.h"  // MatchInfo

class QTableWidget;
class QPushButton;
class QLineEdit;
class QSpinBox;
class QLabel;

class LobbyWidget: public QWidget {
    Q_OBJECT

 public:
    explicit LobbyWidget(QWidget* parent = nullptr);

    void set_matches(const std::vector<MatchInfo>& matches);

    void show_error(const QString& message);

 signals:
    void refreshRequested();
    void createMatchRequested(const QString& name, uint8_t max_players);
    void joinMatchRequested(uint32_t match_id);
    void logoutRequested();

 private slots:
    void on_refresh_clicked();
    void on_create_clicked();
    void on_join_clicked();
    void on_logout_clicked();

 private:
    QTableWidget* table;
    QPushButton* refresh_button;
    QPushButton* join_button;
    QLineEdit* new_name_input;
    QSpinBox* new_max_input;
    QPushButton* create_button;
    QPushButton* logout_button;
    QLabel* status_label;
};

#endif
