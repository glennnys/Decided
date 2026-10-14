#include "mngactivity.h"
#include "ui_mngactivity.h"

MngActivity::MngActivity(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MngActivity)
{
    ui->setupUi(this);
    connect(ui->confirm_item_btn, &QPushButton::clicked, this, &MngActivity::add_item);
    connect(ui->cancel_item_btn, &QPushButton::clicked, this, &MngActivity::cancel_item);
    connect(ui->delete_item_btn, &QPushButton::clicked, this, &MngActivity::delete_item);
    connect(ui->save_changes, &QPushButton::clicked, this, &MngActivity::save_changes);
    connect(ui->cancel_changes, &QPushButton::clicked, this, &MngActivity::cancel_changes);
    connect(ui->existing_item_list, &QListWidget::itemDoubleClicked, this, &MngActivity::edit_item);

    load_items();
}

MngActivity::~MngActivity()
{
    delete ui;
}

//Item creation
void MngActivity::cancel_changes()
{
    new_item_list = cur_item_list;

    set_item_display();
}

void MngActivity::save_changes()
{
    QSqlDatabase activityDb = QSqlDatabase::database("activity");
    QSqlQuery query(activityDb);

    QList<int> toAdd;     // items to insert in DB
    QList<int> toRemove;  // items to delete from DB

    // Detect additions and updates
    for (auto key : new_item_list.keys()) {
        if (!cur_item_list.contains(key)) {
            toAdd.append(key); // new local item
        }
        else if (new_item_list[key] != cur_item_list[key]) {
            toAdd.append(key); // edited local item
        }
    }

    // Detect removals
    for (auto key : cur_item_list.keys()) {
        if (!new_item_list.contains(key)) {
            toRemove.append(key);
        }
    }

    activityDb.transaction();

    query.prepare("DELETE FROM Items "
                  "WHERE id = :id");

    for (const auto &id : std::as_const(toRemove)) {
        qDebug() << "Removing:" << cur_item_list[id].name << "," << cur_item_list[id].description;

        query.bindValue(":id", id);
        if (!query.exec()) {
            qInfo() << "Deletion failed:" << query.lastError();
        }
    }

    query.prepare("INSERT OR REPLACE INTO Items (id, name, description)"
                  "VALUES (:id, :name, :desc)");

    for (const auto &id : std::as_const(toAdd)) {
        qDebug() << "Adding:" << new_item_list[id].name << "," << new_item_list[id].description;

        query.bindValue(":id", id);
        query.bindValue(":name", new_item_list[id].name);
        query.bindValue(":desc", new_item_list[id].description);
        if (!query.exec()) {
            qInfo() << "Insert failed:" << query.lastError();
        }
    }

    activityDb.commit(); // commit all inserts at once

    cur_item_list = new_item_list;
}

void MngActivity::load_items()
{
    QSqlDatabase activityDb = QSqlDatabase::database("activity");

    QSqlQuery query(activityDb);

    if (!query.exec("SELECT id, name, description FROM Items")) {
        qWarning() << "Query failed:" << query.lastError();
    } else {
        while (query.next()) {
            cur_item_list.insert(query.value(0).toInt(), {query.value(1).toString(), query.value(2).toString()});
        }
    }

    new_item_list = cur_item_list;

    set_item_display();
}

void MngActivity::set_item_display()
{
    ui->selected_item_list->clear();
    ui->existing_item_list->clear();

    for (auto &item : new_item_list.values()){
        // if (item.selected & (item.name.toLower().contains(ui->search_item_entry->text().toLower()) || item.description.toLower().contains(ui->search_item_entry->text().toLower()))){
        //     QListWidgetItem *widgetItem = new QListWidgetItem(item.name);
        //     widgetItem->setToolTip(item.description);
        //     ui->selected_item_list->addItem(widgetItem);
        // }
        if (item.name.toLower().contains(ui->search_item_entry->text().toLower()) || item.description.toLower().contains(ui->search_item_entry->text().toLower())){
            QListWidgetItem *widgetItem = new QListWidgetItem(item.name);
            widgetItem->setToolTip(item.description);
            ui->existing_item_list->addItem(widgetItem);
        }
    }

    ui->selected_item_list->sortItems();
    ui->existing_item_list->sortItems();
}

void MngActivity::cancel_item()
{
    ui->item_name_entry->clear();
    ui->item_desc_entry->clear();

    ui->confirm_item_btn->setText("Add item");

    editing_item = -1;
}

void MngActivity::add_item()
{
    int id = editing_item;
    const QString name = ui->item_name_entry->text();
    const QString desc = ui->item_desc_entry->document()->toPlainText();

    if (id>0){
        Item old_item = new_item_list[id];
        Item *item = &new_item_list[id];

        QListWidgetItem *list_item = ui->existing_item_list->findItems(old_item.name, Qt::MatchExactly)[0];

        list_item->setText(name);
        list_item->setToolTip(desc);

        ui->existing_item_list->sortItems();

        item->name = name;
        item->description = desc;
    }
    else {
        if (find_same_name(name, new_item_list) != -1){
            QMessageBox::warning(
                this,
                tr("Simealplified"),
                tr("Unable to add item. Another item with that name already exists.") );
            return;
        }

        QListWidgetItem *item = new QListWidgetItem(name);
        int new_id = generate_id(new_item_list.keys());
        new_item_list.insert(new_id, {name, desc});
        item->setToolTip(desc);
        ui->existing_item_list->addItem(item);
        ui->existing_item_list->sortItems();
    }

    cancel_item();
}

void MngActivity::delete_item()
{
    if (new_item_list.remove(find_same_name(ui->existing_item_list->currentItem()->text(), new_item_list))) {
        ui->existing_item_list->takeItem(ui->existing_item_list->currentRow());
    } else {
        qWarning() << "Couldn't remove item";
    }

    cancel_item();
}

void MngActivity::edit_item(QListWidgetItem *item)
{
    ui->confirm_item_btn->setText("Edit item");

    ui->item_name_entry->setText(item->text());
    ui->item_desc_entry->setText(item->toolTip());

    editing_item = find_same_name(item->text(), new_item_list);
}
