#include "tag.h"
#include "ui_tag.h"

Tag::Tag(QWidget *parent, QString name, int id)
    : QWidget(parent)
    , ui(new Ui::Tag)
{
    ui->setupUi(this);
    ui->tag_btn->setText(name);
    connect(ui->tag_btn, &QPushButton::clicked, this, [this] {emit tag_removed(this);});

    tagName = name;
    tagId = id;
}

Tag::~Tag()
{
    delete ui;
}