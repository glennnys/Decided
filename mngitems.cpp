#include "mngitems.h"
#include "ui_mngitems.h"

MngItems::MngItems(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MngItems)
{
    ui->setupUi(this);

    connect(ui->save_btn, &QPushButton::clicked, this, &MngItems::save_changes);
    connect(ui->cancel_btn, &QPushButton::clicked, this, &MngItems::cancel_changes);

    editing_item = -1;
    connect(ui->search_item_entry, &QLineEdit::textChanged, this, &MngItems::set_item_display);
    connect(ui->confirm_item_btn, &QPushButton::clicked, this, &MngItems::add_item);
    connect(ui->cancel_item_btn, &QPushButton::clicked, this, &MngItems::cancel_item);
    connect(ui->delete_item_btn, &QPushButton::clicked, this, &MngItems::delete_item);
    connect(ui->existing_item_list, &QListWidget::itemClicked, this, &MngItems::edit_item);
    connect(ui->selected_item_list, &QListWidget::itemClicked, this, &MngItems::edit_item);
    connect(ui->existing_item_list, &QListWidget::itemDoubleClicked, this, &MngItems::select_item);
    connect(ui->selected_item_list, &QListWidget::itemDoubleClicked, this, &MngItems::deselect_item);

    connect(ui->add_item_btn,    &QPushButton::clicked, this, &MngItems::select_item);
    connect(ui->remove_item_btn, &QPushButton::clicked, this, &MngItems::deselect_item);
    connect(ui->reset_item_list, &QPushButton::clicked, this, &MngItems::reset_selection);

    load_items();
    set_item_display();
}

MngItems::~MngItems()
{
    delete ui;
}

void MngItems::cancel_changes()
{
    new_item_list = cur_item_list;

    set_item_display();
}

void MngItems::save_changes()
{
    QSqlDatabase activityDb = QSqlDatabase::database("activity");
    QSqlQuery query(activityDb);

    if (!query.exec("PRAGMA foreign_keys = ON")) {
        qWarning() << "Failed to enable foreign keys:"
                   << query.lastError();
    }

    //Items
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
        query.bindValue(":id", id);

        if (!query.exec()) {
            qInfo() << "Deletion failed:" << query.lastError();
            activityDb.rollback();
            return;
        }
    }

    query.prepare("INSERT OR REPLACE INTO Items (id, name, description)"
                  "VALUES (:id, :name, :desc)");

    for (const auto &id : std::as_const(toAdd)) {
        query.bindValue(":id", id);
        query.bindValue(":name", new_item_list[id].name);
        query.bindValue(":desc", new_item_list[id].description);

        if (!query.exec()) {
            qInfo() << "Insert failed:" << query.lastError();
            activityDb.rollback();
            return;
        }
    }



    activityDb.commit(); // commit all inserts at once

    cur_item_list = new_item_list;
}

void MngItems::set_item_display()
{
    ui->selected_item_list->clear();
    ui->existing_item_list->clear();

    for (auto [id, item] : new_item_list.asKeyValueRange())
    {
        if (item.name.toLower().contains(ui->search_item_entry->text().toLower()) || item.description.toLower().contains(ui->search_item_entry->text().toLower()))
        {
            QListWidgetItem *widgetItem = new QListWidgetItem(item.name);
            widgetItem->setToolTip(item.description);
            if (selected_items.contains(id)) ui->selected_item_list->addItem(widgetItem);
            else ui->existing_item_list->addItem(widgetItem);
        }
    }

    ui->selected_item_list->sortItems();
    ui->existing_item_list->sortItems();
}

void MngItems::load_items()
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
}

void MngItems::cancel_item()
{
    ui->item_name_entry->clear();
    ui->item_desc_entry->clear();

    ui->confirm_item_btn->setText("Add item");

    editing_item = -1;

    set_item_display();
}

void MngItems::add_item()
{
    const int id = editing_item;
    const QString name = ui->item_name_entry->text();
    const QString desc = ui->item_desc_entry->document()->toPlainText();

    if (id>0)
    {
        auto item = new_item_list.find(id);
        if (item == new_item_list.end())
            return;

        bool selected = selected_items.contains(id);
        QListWidgetItem *list_item = selected ? ui->selected_item_list->findItems(item->name, Qt::MatchExactly)[0] : ui->existing_item_list->findItems(item->name, Qt::MatchExactly)[0];

        item->name = name;
        item->description = desc;


        if (list_item)
        {
            list_item->setText(name);
            list_item->setToolTip(desc);
        }

        if (selected) ui->selected_item_list->sortItems();
        else ui->existing_item_list->sortItems();

    } else {
        if (find_same_name(name, new_item_list) != -1){
            QMessageBox::warning(
                this,
                tr("Simealplified"),
                tr("Unable to add item. Another item with that name already exists.") );
            return;
        }

        Item item(name, desc);

        const int new_id = generate_id(cur_item_list.keys(), new_item_list.keys());
        new_item_list.insert(new_id, item);

        auto *list_item = new QListWidgetItem(name);
        list_item->setToolTip(desc);
        ui->existing_item_list->addItem(list_item);

        ui->existing_item_list->sortItems();
    }

    cancel_item();
}

void MngItems::delete_item()
{
    if (new_item_list.remove(find_same_name(ui->existing_item_list->currentItem()->text(), new_item_list))) {
        ui->existing_item_list->takeItem(ui->existing_item_list->currentRow());
    } else {
        qWarning() << "Couldn't remove item";
    }

    cancel_item();
}

void MngItems::edit_item(QListWidgetItem *list_item)
{
    int item_id = find_same_name(list_item->text(), new_item_list);

    if (item_id < 0) return;
    ui->confirm_item_btn->setText("Edit item");

    ui->item_name_entry->setText(new_item_list[item_id].name);
    ui->item_desc_entry->setText(new_item_list[item_id].description);

    editing_item = item_id;
}

void MngItems::select_item()
{
    if (!ui->existing_item_list->currentItem()) return;

    // remove item from unselected list view and list
    auto selected_item = ui->existing_item_list->takeItem(ui->existing_item_list->currentRow());
    int item_id = find_same_name(selected_item->text(), new_item_list);

    // add item to selected list view
    ui->selected_item_list->addItem(selected_item);
    ui->selected_item_list->sortItems();

    // add item to selected list
    selected_items.insert(item_id);

    set_item_display();
}

void MngItems::deselect_item()
{
    if (!ui->selected_item_list->currentItem()) return;

    // remove item from selected list view and list
    auto deselected_item = ui->selected_item_list->takeItem(ui->selected_item_list->currentRow());
    int item_id = find_same_name(deselected_item->text(), new_item_list);

    // add item to existing list view
    ui->existing_item_list->addItem(deselected_item);
    ui->existing_item_list->sortItems();

    // remove item from selected list
    selected_items.remove(item_id);

    set_item_display();
}

void MngItems::reset_selection()
{
    ui->selected_item_list->clear();

    selected_items.clear();

    set_item_display();
}
