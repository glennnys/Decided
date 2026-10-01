#ifndef GENACTIVITY_H
#define GENACTIVITY_H

#include <QWidget>

namespace Ui {
class GenActivity;
}

class GenActivity : public QWidget
{
    Q_OBJECT

public:
    explicit GenActivity(QWidget *parent = nullptr);
    ~GenActivity();

private:
    Ui::GenActivity *ui;
};

#endif // GENACTIVITY_H
