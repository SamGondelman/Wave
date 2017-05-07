#include "InstantGestureDialog.h"
#include "ui_InstantGestureDialog.h"

InstantGestureDialog::InstantGestureDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InstantGestureDialog)
{
    ui->setupUi(this);
}

InstantGestureDialog::~InstantGestureDialog()
{
    delete ui;
}

float InstantGestureDialog::getMargin() {
    return ui->marginSlider->value() / 100.0f;
}

bool InstantGestureDialog::getUsePosition() {
    return !ui->ignorePosition->isChecked();
}

bool InstantGestureDialog::getUseYaw() {
    return !ui->ignoreYaw->isChecked();
}

bool InstantGestureDialog::getUsePitch() {
    return !ui->ignorePitch->isChecked();
}

bool InstantGestureDialog::getUseRoll() {
    return !ui->ignoreRoll->isChecked();
}

const QString InstantGestureDialog::getName() {
    return ui->nameEdit->text();
}
