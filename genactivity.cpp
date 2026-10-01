#include "genactivity.h"
#include "ui_genactivity.h"

GenActivity::GenActivity(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GenActivity)
{
    ui->setupUi(this);
}

GenActivity::~GenActivity()
{
    delete ui;
}
