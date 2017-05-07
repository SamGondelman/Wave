#ifndef INSTANTGESTUREDIALOG_H
#define INSTANTGESTUREDIALOG_H

#include <QDialog>

namespace Ui {
class InstantGestureDialog;
}

class InstantGestureDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InstantGestureDialog(QWidget *parent = 0);
    ~InstantGestureDialog();

    float getMargin();
    bool getUsePosition();
    bool getUseYaw();
    bool getUsePitch();
    bool getUseRoll();
    const QString getName();

private:
    Ui::InstantGestureDialog *ui;
};

#endif // INSTANTGESTUREDIALOG_H
