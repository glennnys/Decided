#include "selectedactivity.h"
#include "ui_selectedactivity.h"

SelectedActivity::SelectedActivity(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SelectedActivity)
{
    ui->setupUi(this);
}

SelectedActivity::~SelectedActivity()
{
    delete ui;
}
