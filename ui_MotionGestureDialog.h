/********************************************************************************
** Form generated from reading UI file 'MotionGestureDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MOTIONGESTUREDIALOG_H
#define UI_MOTIONGESTUREDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSlider>

QT_BEGIN_NAMESPACE

class Ui_MotionGestureDialog
{
public:
    QDialogButtonBox *buttonBox;
    QSlider *marginSlider;
    QLabel *marginLabel;
    QCheckBox *ignorePosition;
    QCheckBox *ignoreYaw;
    QCheckBox *ignorePitch;
    QCheckBox *ignoreRoll;
    QLabel *nameLabel;
    QLineEdit *nameEdit;
    QSlider *durationSlider;
    QLabel *durationLabel;

    void setupUi(QDialog *MotionGestureDialog)
    {
        if (MotionGestureDialog->objectName().isEmpty())
            MotionGestureDialog->setObjectName(QStringLiteral("MotionGestureDialog"));
        MotionGestureDialog->resize(400, 310);
        buttonBox = new QDialogButtonBox(MotionGestureDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setGeometry(QRect(30, 270, 341, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        marginSlider = new QSlider(MotionGestureDialog);
        marginSlider->setObjectName(QStringLiteral("marginSlider"));
        marginSlider->setGeometry(QRect(100, 30, 241, 20));
        marginSlider->setMaximum(100);
        marginSlider->setValue(10);
        marginSlider->setSliderPosition(10);
        marginSlider->setOrientation(Qt::Horizontal);
        marginLabel = new QLabel(MotionGestureDialog);
        marginLabel->setObjectName(QStringLiteral("marginLabel"));
        marginLabel->setGeometry(QRect(20, 30, 59, 17));
        QFont font;
        font.setPointSize(14);
        marginLabel->setFont(font);
        ignorePosition = new QCheckBox(MotionGestureDialog);
        ignorePosition->setObjectName(QStringLiteral("ignorePosition"));
        ignorePosition->setGeometry(QRect(100, 90, 91, 23));
        QFont font1;
        font1.setPointSize(13);
        ignorePosition->setFont(font1);
        ignorePosition->setChecked(true);
        ignoreYaw = new QCheckBox(MotionGestureDialog);
        ignoreYaw->setObjectName(QStringLiteral("ignoreYaw"));
        ignoreYaw->setGeometry(QRect(100, 170, 91, 23));
        ignoreYaw->setFont(font1);
        ignorePitch = new QCheckBox(MotionGestureDialog);
        ignorePitch->setObjectName(QStringLiteral("ignorePitch"));
        ignorePitch->setGeometry(QRect(100, 130, 91, 23));
        ignorePitch->setFont(font1);
        ignoreRoll = new QCheckBox(MotionGestureDialog);
        ignoreRoll->setObjectName(QStringLiteral("ignoreRoll"));
        ignoreRoll->setGeometry(QRect(100, 210, 91, 23));
        ignoreRoll->setFont(font1);
        nameLabel = new QLabel(MotionGestureDialog);
        nameLabel->setObjectName(QStringLiteral("nameLabel"));
        nameLabel->setGeometry(QRect(20, 240, 59, 17));
        nameLabel->setFont(font);
        nameEdit = new QLineEdit(MotionGestureDialog);
        nameEdit->setObjectName(QStringLiteral("nameEdit"));
        nameEdit->setGeometry(QRect(100, 240, 241, 20));
        durationSlider = new QSlider(MotionGestureDialog);
        durationSlider->setObjectName(QStringLiteral("durationSlider"));
        durationSlider->setGeometry(QRect(100, 60, 241, 20));
        durationSlider->setMaximum(100);
        durationSlider->setValue(50);
        durationSlider->setSliderPosition(50);
        durationSlider->setOrientation(Qt::Horizontal);
        durationLabel = new QLabel(MotionGestureDialog);
        durationLabel->setObjectName(QStringLiteral("durationLabel"));
        durationLabel->setGeometry(QRect(20, 60, 71, 17));
        durationLabel->setFont(font);

        retranslateUi(MotionGestureDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), MotionGestureDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), MotionGestureDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(MotionGestureDialog);
    } // setupUi

    void retranslateUi(QDialog *MotionGestureDialog)
    {
        MotionGestureDialog->setWindowTitle(QApplication::translate("MotionGestureDialog", "Dialog", 0));
        marginLabel->setText(QApplication::translate("MotionGestureDialog", "Error Margin:", 0));
        ignorePosition->setText(QApplication::translate("MotionGestureDialog", "Ignore Position?", 0));
        ignoreYaw->setText(QApplication::translate("MotionGestureDialog", "Ignore Yaw?", 0));
        ignorePitch->setText(QApplication::translate("MotionGestureDialog", "Ignore Pitch?", 0));
        ignoreRoll->setText(QApplication::translate("MotionGestureDialog", "Ignore Roll?", 0));
        nameLabel->setText(QApplication::translate("MotionGestureDialog", "Name:", 0));
        nameEdit->setText(QString());
        durationLabel->setText(QApplication::translate("MotionGestureDialog", "Frame Duration:", 0));
    } // retranslateUi

};

namespace Ui {
    class MotionGestureDialog: public Ui_MotionGestureDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MOTIONGESTUREDIALOG_H
