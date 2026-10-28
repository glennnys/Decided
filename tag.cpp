#include "tag.h"
#include "ui_tag.h"

Tag::Tag(QWidget *parent, QString name, int id, bool showToggle)
    : QWidget(parent)
    , ui(new Ui::Tag)
{
    ui->setupUi(this);
    ui->tag_btn->setText(name);
    connect(ui->tag_btn, &QPushButton::clicked, this, [this] {emit tag_removed(this);});

    tagName = name;
    tagId = id;

    auto *layout = ui->tagLayout;
    layout->setSpacing(0);

    if (showToggle) {
        toggleBtn = new QCheckBox(this);
        layout->insertWidget(0, toggleBtn);
        connect(toggleBtn, &QCheckBox::checkStateChanged, this, [this] {emit toggled(tagId, toggleBtn->isChecked());});
    }

}

Tag::~Tag()
{
    delete ui;
}