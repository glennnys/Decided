#ifndef MNGACTIVITY_H
#define MNGACTIVITY_H

#include <QWidget>

namespace Ui {
class MngActivity;
}

class MngActivity : public QWidget
{
    Q_OBJECT

public:
    explicit MngActivity(QWidget *parent = nullptr);
    ~MngActivity();

private:
    Ui::MngActivity *ui;
};

#endif // MNGACTIVITY_H
