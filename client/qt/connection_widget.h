#ifndef CONNECTION_WIDGET_H
#define CONNECTION_WIDGET_H

#include <QString>
#include <QWidget>

class QLineEdit;
class QPushButton;
class QLabel;

class ConnectionWidget: public QWidget {
    Q_OBJECT

 public:
    explicit ConnectionWidget(QWidget* parent = nullptr);

    void set_busy(bool busy);

    void show_error(const QString& message);

 signals:
    void connectRequested(const QString& host, const QString& port, const QString& nick);

 private slots:
    void on_connect_clicked();

 private:
    QLineEdit* host_input;
    QLineEdit* port_input;
    QLineEdit* nick_input;
    QPushButton* connect_button;
    QLabel* error_label;
};

#endif
