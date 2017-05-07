#include "MotionGestureDialog.h"
#include "ui_MotionGestureDialog.h"

MotionGestureDialog::MotionGestureDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MotionGestureDialog)
{
    ui->setupUi(this);
}

MotionGestureDialog::~MotionGestureDialog()
{
    delete ui;
}

float MotionGestureDialog::getMargin() {
    return ui->marginSlider->value() / 100.0f;
}

float MotionGestureDialog::getDuration() {
    return ui->durationSlider->value() / 100.0f;
}

bool MotionGestureDialog::getUsePosition() {
    return !ui->ignorePosition->isChecked();
}

bool MotionGestureDialog::getUseYaw() {
    return !ui->ignoreYaw->isChecked();
}

bool MotionGestureDialog::getUsePitch() {
    return !ui->ignorePitch->isChecked();
}

bool MotionGestureDialog::getUseRoll() {
    return !ui->ignoreRoll->isChecked();
}

const QString MotionGestureDialog::getName() {
    return ui->nameEdit->text();
}
