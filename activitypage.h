#ifndef ACTIVITYPAGE_H
#define ACTIVITYPAGE_H

#include <QDialog>

namespace Ui {
class ActivityPage;
}

class ActivityPage : public QDialog
{
    Q_OBJECT

public:
    explicit ActivityPage(QWidget *parent = nullptr);
    ~ActivityPage();

private:
    Ui::ActivityPage *ui;
};

#endif // ACTIVITYPAGE_H
