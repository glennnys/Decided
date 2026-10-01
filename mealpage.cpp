#include "mealpage.h"
#include "ui_mealpage.h"

MealPage::MealPage(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MealPage)
{
    ui->setupUi(this);
}

MealPage::~MealPage()
{
    delete ui;
}
