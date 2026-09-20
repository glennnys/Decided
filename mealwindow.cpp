#include "mealwindow.h"
#include "ui_mealwindow.h"

MealWindow::MealWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MealWindow)
{
    ui->setupUi(this);
}

MealWindow::~MealWindow()
{
    delete ui;
}
