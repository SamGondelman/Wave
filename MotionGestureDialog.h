#ifndef MOTIONGESTUREDIALOG_H
#define MOTIONGESTUREDIALOG_H

#include <QDialog>

namespace Ui {
class MotionGestureDialog;
}

class MotionGestureDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MotionGestureDialog(QWidget *parent = 0);
    ~MotionGestureDialog();

    float getMargin();
    float getDuration();
    bool getUsePosition();
    bool getUseYaw();
    bool getUsePitch();
    bool getUseRoll();
    const QString getName();

private:
    Ui::MotionGestureDialog *ui;
};

#endif // MOTIONGESTUREDIALOG_H
