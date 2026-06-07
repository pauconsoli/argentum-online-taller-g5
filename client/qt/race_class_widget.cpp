#include "race_class_widget.h"

#include <QComboBox>
#include <QFont>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

RaceClassWidget::RaceClassWidget(QWidget* parent):
        QWidget(parent),
        race_combo(new QComboBox(this)),
        class_combo(new QComboBox(this)),
        confirm_button(new QPushButton(tr("CONFIRMAR"), this)),
        back_button(new QPushButton(tr("← Volver al lobby"), this)) {

    race_combo->addItems({tr("Humano"), tr("Elfo"), tr("Enano"), tr("Gnomo")});
    class_combo->addItems({tr("Mago"), tr("Clérigo"), tr("Paladín"), tr("Guerrero")});

    auto* title = new QLabel(tr("Elegí tu personaje"), this);
    title->setAlignment(Qt::AlignCenter);
    {
        QFont f = title->font();
        f.setPointSize(22);
        f.setBold(true);
        title->setFont(f);
    }

    auto* form = new QFormLayout;
    form->addRow(tr("Raza:"), race_combo);
    form->addRow(tr("Clase:"), class_combo);

    confirm_button->setMinimumHeight(45);
    {
        QFont f = confirm_button->font();
        f.setPointSize(14);
        f.setBold(true);
        confirm_button->setFont(f);
    }

    back_button->setStyleSheet(
            "QPushButton { background: transparent; border: 1px solid #8a6a2f;"
            "color: #c89b3c; font-weight: normal; }"
            "QPushButton:hover { background-color: #3a2618; }");

    auto* bottom = new QHBoxLayout;
    bottom->addWidget(back_button);
    bottom->addStretch();

    auto* root = new QVBoxLayout(this);
    root->addLayout(bottom);
    root->addStretch();
    root->addWidget(title);
    root->addSpacing(30);
    root->addLayout(form);
    root->addSpacing(20);
    root->addWidget(confirm_button);
    root->addStretch();
    root->setContentsMargins(80, 20, 80, 40);

    connect(confirm_button, &QPushButton::clicked, this, &RaceClassWidget::on_confirm_clicked);
    connect(back_button, &QPushButton::clicked, this, &RaceClassWidget::on_back_clicked);
}

void RaceClassWidget::on_confirm_clicked() {
    emit confirmRequested(static_cast<uint8_t>(race_combo->currentIndex()),
                          static_cast<uint8_t>(class_combo->currentIndex()));
}

void RaceClassWidget::on_back_clicked() { emit backToLobbyRequested(); }
