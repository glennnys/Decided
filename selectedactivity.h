#ifndef SELECTEDACTIVITY_H
#define SELECTEDACTIVITY_H

#include <QDialog>
#include "utils.h"

namespace Ui {
class SelectedActivity;
}

class SelectedActivity : public QDialog
{
    Q_OBJECT

public:
    explicit SelectedActivity(QWidget *parent = nullptr);
    ~SelectedActivity();

private:
    Ui::SelectedActivity *ui;

private slots:
    void fillActivity(Activity selectedActivity);
};

#endif // SELECTEDACTIVITY_H
