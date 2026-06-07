#include "connection_widget.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

ConnectionWidget::ConnectionWidget(QWidget* parent):
        QWidget(parent),
        host_input(new QLineEdit("127.0.0.1", this)),
        port_input(new QLineEdit("47474", this)),
        nick_input(new QLineEdit(this)),
        connect_button(new QPushButton(tr("Conectar"), this)),
        error_label(new QLabel(this)) {

    nick_input->setPlaceholderText(tr("nombre de jugador"));

    error_label->setStyleSheet("color: #c00;");
    error_label->setVisible(false);
    error_label->setWordWrap(true);

    auto* form = new QFormLayout;
    form->addRow(tr("Host"), host_input);
    form->addRow(tr("Puerto"), port_input);
    form->addRow(tr("Nick"), nick_input);

    auto* root = new QVBoxLayout(this);
    root->addStretch();
    root->addLayout(form);
    root->addWidget(connect_button);
    root->addWidget(error_label);
    root->addStretch();

    connect(connect_button, &QPushButton::clicked, this, &ConnectionWidget::on_connect_clicked);
    // Enter en el nick también dispara connect.
    connect(nick_input, &QLineEdit::returnPressed, this, &ConnectionWidget::on_connect_clicked);
}

void ConnectionWidget::on_connect_clicked() {
    const QString host = host_input->text().trimmed();
    const QString port = port_input->text().trimmed();
    const QString nick = nick_input->text().trimmed();

    if (host.isEmpty() || port.isEmpty() || nick.isEmpty()) {
        show_error(tr("Completá host, puerto y nick."));
        return;
    }

    error_label->setVisible(false);
    emit connectRequested(host, port, nick);
}

void ConnectionWidget::set_busy(bool busy) {
    connect_button->setEnabled(!busy);
    host_input->setEnabled(!busy);
    port_input->setEnabled(!busy);
    nick_input->setEnabled(!busy);
}

void ConnectionWidget::show_error(const QString& message) {
    error_label->setText(message);
    error_label->setVisible(true);
}
