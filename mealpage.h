#ifndef MEALPAGE_H
#define MEALPAGE_H

#include <QDialog>

namespace Ui {
class MealPage;
}

class MealPage : public QDialog
{
    Q_OBJECT

public:
    explicit MealPage(QWidget *parent = nullptr);
    ~MealPage();

private:
    Ui::MealPage *ui;
};

#endif // MEALPAGE_H
