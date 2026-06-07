#include "race_class_widget.h"

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

RaceClassWidget::RaceClassWidget(QWidget* parent):
        QWidget(parent),
        race_combo(new QComboBox(this)),
        class_combo(new QComboBox(this)),
        confirm_button(new QPushButton(tr("Confirmar"), this)) {

    race_combo->addItems({tr("Humano"), tr("Elfo"), tr("Enano"), tr("Gnomo")});
    class_combo->addItems({tr("Mago"), tr("Clérigo"), tr("Paladín"), tr("Guerrero")});

    auto* form = new QFormLayout;
    form->addRow(tr("Raza:"), race_combo);
    form->addRow(tr("Clase:"), class_combo);

    auto* root = new QVBoxLayout(this);
    root->addStretch();
    root->addWidget(new QLabel(tr("Elegí tu personaje"), this));
    root->addLayout(form);
    root->addWidget(confirm_button);
    root->addStretch();

    connect(confirm_button, &QPushButton::clicked, this, &RaceClassWidget::on_confirm_clicked);
}

void RaceClassWidget::on_confirm_clicked() {
    emit confirmRequested(static_cast<uint8_t>(race_combo->currentIndex()),
                          static_cast<uint8_t>(class_combo->currentIndex()));
}
