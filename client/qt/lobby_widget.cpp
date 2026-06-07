#include "lobby_widget.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

LobbyWidget::LobbyWidget(QWidget* parent):
        QWidget(parent),
        table(new QTableWidget(0, 4, this)),
        refresh_button(new QPushButton(tr("Refrescar"), this)),
        join_button(new QPushButton(tr("Unirse"), this)),
        new_name_input(new QLineEdit(this)),
        new_max_input(new QSpinBox(this)),
        create_button(new QPushButton(tr("Crear partida"), this)),
        status_label(new QLabel(this)) {

    // Tabla de partidas
    table->setHorizontalHeaderLabels({tr("ID"), tr("Nombre"), tr("Jugadores"), tr("Max")});
    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->verticalHeader()->setVisible(false);

    // Sección "crear partida"
    new_name_input->setPlaceholderText(tr("nombre partida"));
    new_max_input->setRange(2, 8);
    new_max_input->setValue(4);

    auto* create_box = new QGroupBox(tr("Crear partida nueva"), this);
    auto* create_row = new QHBoxLayout(create_box);
    create_row->addWidget(new QLabel(tr("Nombre:"), this));
    create_row->addWidget(new_name_input, 1);
    create_row->addWidget(new QLabel(tr("Max:"), this));
    create_row->addWidget(new_max_input);
    create_row->addWidget(create_button);

    // Botones de la tabla
    auto* table_buttons = new QHBoxLayout;
    table_buttons->addWidget(refresh_button);
    table_buttons->addStretch();
    table_buttons->addWidget(join_button);

    status_label->setStyleSheet("color: #c00;");
    status_label->setVisible(false);
    status_label->setWordWrap(true);

    auto* root = new QVBoxLayout(this);
    root->addWidget(new QLabel(tr("Partidas disponibles:"), this));
    root->addWidget(table, 1);
    root->addLayout(table_buttons);
    root->addWidget(create_box);
    root->addWidget(status_label);

    connect(refresh_button, &QPushButton::clicked, this, &LobbyWidget::on_refresh_clicked);
    connect(join_button, &QPushButton::clicked, this, &LobbyWidget::on_join_clicked);
    connect(create_button, &QPushButton::clicked, this, &LobbyWidget::on_create_clicked);
    connect(table, &QTableWidget::cellDoubleClicked, this, [this](int, int) { on_join_clicked(); });
}

void LobbyWidget::set_matches(const std::vector<MatchInfo>& matches) {
    table->setRowCount(static_cast<int>(matches.size()));
    int row = 0;
    for (const auto& m: matches) {
        auto* id_item = new QTableWidgetItem(QString::number(m.id));
        // Guardamos el id como UserData para recuperarlo fácil al joinear.
        id_item->setData(Qt::UserRole, m.id);
        table->setItem(row, 0, id_item);
        table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(m.name)));
        table->setItem(row, 2,
                       new QTableWidgetItem(QString::number(m.current_players)));
        table->setItem(row, 3,
                       new QTableWidgetItem(QString::number(m.max_players)));
        ++row;
    }
    status_label->setVisible(false);
}

void LobbyWidget::show_error(const QString& message) {
    status_label->setText(message);
    status_label->setVisible(true);
}

void LobbyWidget::on_refresh_clicked() { emit refreshRequested(); }

void LobbyWidget::on_create_clicked() {
    const QString name = new_name_input->text().trimmed();
    if (name.isEmpty()) {
        show_error(tr("Poné un nombre para la partida."));
        return;
    }
    status_label->setVisible(false);
    emit createMatchRequested(name, static_cast<uint8_t>(new_max_input->value()));
}

void LobbyWidget::on_join_clicked() {
    const int row = table->currentRow();
    if (row < 0) {
        show_error(tr("Seleccioná una partida primero."));
        return;
    }
    auto* item = table->item(row, 0);
    if (!item) {
        return;
    }
    const uint32_t id = item->data(Qt::UserRole).toUInt();
    status_label->setVisible(false);
    emit joinMatchRequested(id);
}
