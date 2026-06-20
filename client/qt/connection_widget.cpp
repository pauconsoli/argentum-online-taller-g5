#include "connection_widget.h"

#include <QFont>
#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

ConnectionWidget::ConnectionWidget(QWidget* parent):
    QWidget(parent),
    host_input(new QLineEdit("127.0.0.1", this)),
    port_input(new QLineEdit("8080", this)),
    nick_input(new QLineEdit(this)),
    connect_button(new QPushButton(tr("ENTRAR"), this)),
    error_label(new QLabel(this)),
    options_toggle(new QToolButton(this)),
    options_panel(new QFrame(this)) {

    auto* logo = new QLabel(this);
    logo->setAlignment(Qt::AlignCenter);
    {
        QPixmap pix(":/logo.png");
        if (!pix.isNull()) {
            logo->setPixmap(
                pix.scaledToHeight(220, Qt::SmoothTransformation));
        } else {
            logo->setText(tr("ARGENTUM ONLINE"));
            QFont f = logo->font();
            f.setPointSize(36);
            f.setBold(true);
            logo->setFont(f);
            logo->setStyleSheet("color: #f0c870; letter-spacing: 4px;");
        }
    }

    auto* subtitle = new QLabel(tr("— G5 —"), this);
    subtitle->setAlignment(Qt::AlignCenter);
    {
        QFont f = subtitle->font();
        f.setPointSize(14);
        f.setItalic(true);
        subtitle->setFont(f);
        subtitle->setStyleSheet("color: #c89b3c;");
    }

    // ===== Campo nick =====
    nick_input->setPlaceholderText(tr("tu nombre..."));
    nick_input->setAlignment(Qt::AlignCenter);
    nick_input->setMinimumHeight(40);
    {
        QFont f = nick_input->font();
        f.setPointSize(18);
        nick_input->setFont(f);
    }

    connect_button->setMinimumHeight(50);
    {
        QFont f = connect_button->font();
        f.setPointSize(18);
        f.setBold(true);
        connect_button->setFont(f);
    }

    error_label->setStyleSheet("color: #ff6b6b; font-weight: bold;");
    error_label->setAlignment(Qt::AlignCenter);
    error_label->setVisible(false);
    error_label->setWordWrap(true);

    options_toggle->setText(tr("  Opciones de conexión"));
    options_toggle->setCheckable(true);
    options_toggle->setChecked(false);
    options_toggle->setArrowType(Qt::RightArrow);
    options_toggle->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    options_toggle->setStyleSheet(
        "QToolButton { background: transparent; border: none; color: #c89b3c; }");

    options_panel->setFrameShape(QFrame::StyledPanel);
    options_panel->setVisible(false);
    auto* opt_form = new QFormLayout(options_panel);
    opt_form->addRow(tr("Host:"), host_input);
    opt_form->addRow(tr("Puerto:"), port_input);

    connect(options_toggle, &QToolButton::toggled, this,
            &ConnectionWidget::on_toggle_advanced_options);

    auto* root = new QVBoxLayout(this);
    root->addStretch(2);
    root->addWidget(logo);
    root->addWidget(subtitle);
    root->addSpacing(30);
    root->addWidget(nick_input);
    root->addSpacing(12);
    root->addWidget(connect_button);
    root->addWidget(error_label);
    root->addStretch(1);
    root->addWidget(options_toggle, 0, Qt::AlignLeft);
    root->addWidget(options_panel);
    root->setContentsMargins(80, 40, 80, 40);

    connect(connect_button, &QPushButton::clicked, this, &ConnectionWidget::on_connect_clicked);
    connect(nick_input, &QLineEdit::returnPressed, this, &ConnectionWidget::on_connect_clicked);
}

void ConnectionWidget::on_toggle_advanced_options(bool checked) {
    options_panel->setVisible(checked);
    options_toggle->setArrowType(checked ? Qt::DownArrow : Qt::RightArrow);
}

void ConnectionWidget::on_connect_clicked() {
    const QString host = host_input->text().trimmed();
    const QString port = port_input->text().trimmed();
    const QString nick = nick_input->text().trimmed();

    if (nick.isEmpty()) {
        show_error(tr("Ingresá tu nombre."));
        return;
    }
    if (host.isEmpty() || port.isEmpty()) {
        show_error(tr("Host o puerto inválidos. Mirá las opciones de conexión."));
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
