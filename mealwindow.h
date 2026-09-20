#ifndef MEALWINDOW_H
#define MEALWINDOW_H

#include <QDialog>

namespace Ui {
class MealWindow;
}

class MealWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MealWindow(QWidget *parent = nullptr);
    ~MealWindow();

private:
    Ui::MealWindow *ui;
};

#endif // MEALWINDOW_H
