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

void MngActivity::load_items()
{
    QSqlDatabase activityDb = QSqlDatabase::database("activity");

    QSqlQuery query(activityDb);

    if (!query.exec("SELECT id, name, description FROM Items")) {
        qWarning() << "Query failed:" << query.lastError();
    } else {
        while (query.next()) {
            cur_item_list.insert(query.value(0).toInt(), {false, query.value(1).toString(), query.value(0).toString()});
        }
    }

    new_item_list = cur_item_list;
}