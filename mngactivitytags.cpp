#include "mngactivitytags.h"
#include "ui_mngactivitytags.h"

MngActivityTags::MngActivityTags(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MngActivityTags)
{
    ui->setupUi(this);
}

MngActivityTags::~MngActivityTags()
{
    delete ui;
}
