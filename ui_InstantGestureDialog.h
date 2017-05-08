/********************************************************************************
** Form generated from reading UI file 'InstantGestureDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INSTANTGESTUREDIALOG_H
#define UI_INSTANTGESTUREDIALOG_H

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

class Ui_InstantGestureDialog
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

    void setupUi(QDialog *InstantGestureDialog)
    {
        if (InstantGestureDialog->objectName().isEmpty())
            InstantGestureDialog->setObjectName(QStringLiteral("InstantGestureDialog"));
        InstantGestureDialog->resize(400, 300);
        buttonBox = new QDialogButtonBox(InstantGestureDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setGeometry(QRect(30, 240, 341, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        marginSlider = new QSlider(InstantGestureDialog);
        marginSlider->setObjectName(QStringLiteral("marginSlider"));
        marginSlider->setGeometry(QRect(100, 30, 241, 20));
        marginSlider->setMaximum(100);
        marginSlider->setValue(10);
        marginSlider->setSliderPosition(10);
        marginSlider->setOrientation(Qt::Horizontal);
        marginLabel = new QLabel(InstantGestureDialog);
        marginLabel->setObjectName(QStringLiteral("marginLabel"));
        marginLabel->setGeometry(QRect(20, 30, 59, 17));
        QFont font;
        font.setPointSize(14);
        marginLabel->setFont(font);
        ignorePosition = new QCheckBox(InstantGestureDialog);
        ignorePosition->setObjectName(QStringLiteral("ignorePosition"));
        ignorePosition->setGeometry(QRect(100, 60, 91, 23));
        QFont font1;
        font1.setPointSize(13);
        ignorePosition->setFont(font1);
        ignorePosition->setChecked(true);
        ignoreYaw = new QCheckBox(InstantGestureDialog);
        ignoreYaw->setObjectName(QStringLiteral("ignoreYaw"));
        ignoreYaw->setGeometry(QRect(100, 140, 91, 23));
        ignoreYaw->setFont(font1);
        ignorePitch = new QCheckBox(InstantGestureDialog);
        ignorePitch->setObjectName(QStringLiteral("ignorePitch"));
        ignorePitch->setGeometry(QRect(100, 100, 91, 23));
        ignorePitch->setFont(font1);
        ignoreRoll = new QCheckBox(InstantGestureDialog);
        ignoreRoll->setObjectName(QStringLiteral("ignoreRoll"));
        ignoreRoll->setGeometry(QRect(100, 180, 91, 23));
        ignoreRoll->setFont(font1);
        nameLabel = new QLabel(InstantGestureDialog);
        nameLabel->setObjectName(QStringLiteral("nameLabel"));
        nameLabel->setGeometry(QRect(20, 210, 59, 17));
        nameLabel->setFont(font);
        nameEdit = new QLineEdit(InstantGestureDialog);
        nameEdit->setObjectName(QStringLiteral("nameEdit"));
        nameEdit->setGeometry(QRect(100, 210, 241, 20));

        retranslateUi(InstantGestureDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), InstantGestureDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), InstantGestureDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(InstantGestureDialog);
    } // setupUi

    void retranslateUi(QDialog *InstantGestureDialog)
    {
        InstantGestureDialog->setWindowTitle(QApplication::translate("InstantGestureDialog", "Dialog", 0));
        marginLabel->setText(QApplication::translate("InstantGestureDialog", "Error Margin:", 0));
        ignorePosition->setText(QApplication::translate("InstantGestureDialog", "Ignore Position?", 0));
        ignoreYaw->setText(QApplication::translate("InstantGestureDialog", "Ignore Yaw?", 0));
        ignorePitch->setText(QApplication::translate("InstantGestureDialog", "Ignore Pitch?", 0));
        ignoreRoll->setText(QApplication::translate("InstantGestureDialog", "Ignore Roll?", 0));
        nameLabel->setText(QApplication::translate("InstantGestureDialog", "Name:", 0));
        nameEdit->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class InstantGestureDialog: public Ui_InstantGestureDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INSTANTGESTUREDIALOG_H
