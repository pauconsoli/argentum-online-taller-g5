#ifndef RACE_CLASS_WIDGET_H
#define RACE_CLASS_WIDGET_H

#include <cstdint>

#include <QWidget>

class QComboBox;
class QPushButton;

// RaceClassWidget: tercera pantalla. Después de unirse a un match, el
// jugador elige raza y clase antes de entrar al mundo SDL.
class RaceClassWidget: public QWidget {
    Q_OBJECT

 public:
    explicit RaceClassWidget(QWidget* parent = nullptr);

 signals:
    void confirmRequested(uint8_t race, uint8_t klass);

 private slots:
    void on_confirm_clicked();

 private:
    QComboBox* race_combo;
    QComboBox* class_combo;
    QPushButton* confirm_button;
};

#endif
