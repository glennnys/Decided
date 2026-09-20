#include "activitypage.h"
#include "ui_activitypage.h"

ActivityPage::ActivityPage(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ActivityPage)
{
    ui->setupUi(this);
}

ActivityPage::~ActivityPage()
{
    delete ui;
}
