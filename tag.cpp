#include "tag.h"
#include "ui_tag.h"

Tag::Tag(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Tag)
{
    ui->setupUi(this);
}

Tag::~Tag()
{
    delete ui;
}
