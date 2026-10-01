#include "mngactivity.h"
#include "ui_mngactivity.h"

MngActivity::MngActivity(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MngActivity)
{
    ui->setupUi(this);
}

MngActivity::~MngActivity()
{
    delete ui;
}

//Item creation
void MngActivity::clear()
{
    ui->item_name_entry->clear();
    ui->item_desc_entry->clear();
}

void MngActivity::create_item(bool editing, QString ini_name, QString ini_desc)
{

    const QString name = ui->item_name_entry->text();
    const QString desc = ui->item_desc_entry->document()->toPlainText();
    if (editing)
    {
        emit itemEdited(ini_name, ini_desc, name, desc);
    }
    else
    {
        emit itemCreated(name, desc);
    }
}
