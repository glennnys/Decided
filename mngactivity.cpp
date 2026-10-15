#include "mngactivity.h"
#include "ui_mngactivity.h"

MngActivity::MngActivity(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MngActivity)
{
    ui->setupUi(this);

    connect(ui->save_changes, &QPushButton::clicked, this, &MngActivity::save_changes);
    connect(ui->cancel_changes, &QPushButton::clicked, this, &MngActivity::cancel_changes);

    editing_item = -1;
    connect(ui->search_item_entry, &QLineEdit::textChanged, this, &MngActivity::set_item_display);
    connect(ui->confirm_item_btn, &QPushButton::clicked, this, &MngActivity::add_item);
    connect(ui->cancel_item_btn, &QPushButton::clicked, this, &MngActivity::cancel_item);
    connect(ui->delete_item_btn, &QPushButton::clicked, this, &MngActivity::delete_item);
    connect(ui->existing_item_list, &QListWidget::itemClicked, this, &MngActivity::edit_item);
    connect(ui->selected_item_list, &QListWidget::itemClicked, this, &MngActivity::edit_item);
    connect(ui->existing_item_list, &QListWidget::itemDoubleClicked, this, &MngActivity::select_item);
    connect(ui->selected_item_list, &QListWidget::itemDoubleClicked, this, &MngActivity::deselect_item);

    connect(ui->add_item_btn,    &QPushButton::clicked, this, &MngActivity::select_item);
    connect(ui->remove_item_btn, &QPushButton::clicked, this, &MngActivity::deselect_item);
    connect(ui->reset_item_list, &QPushButton::clicked, this, &MngActivity::reset_selection);

    editing_act = -1;
    connect(ui->search_act_entry, &QLineEdit::textChanged, this, &MngActivity::set_act_display);
    connect(ui->confirm_act_btn, &QPushButton::clicked, this, &MngActivity::add_activity);
    connect(ui->cancel_act_btn, &QPushButton::clicked, this, &MngActivity::cancel_activity);
    connect(ui->delete_act_btn, &QPushButton::clicked, this, &MngActivity::delete_activity);
    connect(ui->act_list, &QListWidget::itemClicked, this, &MngActivity::edit_activity);

    connect(ui->act_min_p_spin, &QSpinBox::valueChanged, ui->act_max_p_spin, &QSpinBox::setMinimum);
    connect(ui->act_min_p_spin, &QSpinBox::valueChanged, ui->act_max_p_slider, &QSlider::setMinimum);

    load_items();
    load_activities();
    set_item_display();
    set_act_display();
}

MngActivity::~MngActivity()
{
    delete ui;
}

//General section
void MngActivity::cancel_changes()
{
    new_item_list = cur_item_list;
    new_act_list = cur_act_list;

    set_item_display();
    set_act_display();
}

void MngActivity::save_changes()
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

    //Activities
    toAdd.clear();
    toRemove.clear();

    // Detect additions and updates
    for (auto key : new_act_list.keys()) {
        if (!cur_act_list.contains(key)) {
            toAdd.append(key); // new local item
        }
        else if (new_act_list[key] != cur_act_list[key]) {
            toAdd.append(key); // edited local item
        }
    }

    // Detect removals
    for (auto key : cur_act_list.keys()) {
        if (!new_act_list.contains(key)) {
            toRemove.append(key);
        }
    }

    activityDb.transaction();

    query.prepare("DELETE FROM Activities "
                  "WHERE id = :id");

    for (const auto &id : std::as_const(toRemove)) {
        query.bindValue(":id", id);

        if (!query.exec()) {
            qInfo() << "Deletion failed:" << query.lastError();
            activityDb.rollback();
            return;
        }
    }

    query.prepare("INSERT OR REPLACE INTO Activities (id, name, description, duration, minPlayers, MaxPlayers)"
                  "VALUES (:id, :name, :desc, :dur, :min, :max)");

    for (const auto &id : std::as_const(toAdd)) {
        query.bindValue(":id", id);
        query.bindValue(":name", new_act_list[id].name);
        query.bindValue(":desc", new_act_list[id].description);
        query.bindValue(":dur", new_act_list[id].duration);
        query.bindValue(":min", new_act_list[id].min_players);
        query.bindValue(":max", new_act_list[id].max_players);

        if (!query.exec()) {
            qInfo() << "Insert failed:" << query.lastError();
            activityDb.rollback();
            return;
        }
    }

    QSqlQuery del_query(activityDb);
    QSqlQuery ins_query(activityDb);

    del_query.prepare("DELETE FROM ActivityRequirements "
                  "WHERE activityId = :act_id AND itemId = :item_id ");

    ins_query.prepare("INSERT INTO ActivityRequirements (activityId, itemId)"
                  "VALUES (:act_id,:item_id)");

    for (const auto act_id : new_act_list.keys()) {
        const auto &new_act = new_act_list[act_id];

        QSet<int> toAdd = new_act.items;
        QSet<int> toRemove;

        auto old_act = cur_act_list.constFind(act_id);

        if (old_act != cur_act_list.cend()) {
            toRemove = old_act->items;

            toRemove.subtract(new_act.items);
            toAdd.subtract(old_act->items);
        }


        for (const auto &item_id : std::as_const(toRemove)) {
            del_query.bindValue(":act_id", act_id);
            del_query.bindValue(":item_id", item_id);

            if (!del_query.exec()) {
                qInfo() << "Deletion failed:" << del_query.lastError();
                activityDb.rollback();
                return;
            }
        }


        for (const auto &item_id : std::as_const(toAdd)) {
            ins_query.bindValue(":act_id", act_id);
            ins_query.bindValue(":item_id", item_id);

            if (!ins_query.exec()) {
                qInfo() << "Insert failed:" << ins_query.lastError();
                activityDb.rollback();
                return;
            }
        }
    }

    activityDb.commit(); // commit all inserts at once

    cur_item_list = new_item_list;
    cur_act_list = new_act_list;
}

// Item section
void MngActivity::set_item_display()
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

void MngActivity::delete_item()
{
    if (new_item_list.remove(find_same_name(ui->existing_item_list->currentItem()->text(), new_item_list))) {
        ui->existing_item_list->takeItem(ui->existing_item_list->currentRow());
    } else {
        qWarning() << "Couldn't remove item";
    }

    cancel_item();
}

void MngActivity::edit_item(QListWidgetItem *list_item)
{
    int item_id = find_same_name(list_item->text(), new_item_list);

    if (item_id < 0) return;
    ui->confirm_item_btn->setText("Edit item");

    ui->item_name_entry->setText(new_item_list[item_id].name);
    ui->item_desc_entry->setText(new_item_list[item_id].description);

    editing_item = item_id;
}

void MngActivity::select_item()
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

void MngActivity::deselect_item()
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

void MngActivity::reset_selection()
{
    ui->selected_item_list->clear();

    selected_items.clear();

    set_item_display();
}

// Activity section

void MngActivity::set_act_display()
{
    ui->act_list->clear();

    for (auto [id, act] : new_act_list.asKeyValueRange())
    {
        if (act.name.toLower().contains(ui->search_act_entry->text().toLower()) || act.description.toLower().contains(ui->search_act_entry->text().toLower()))
        {
            QListWidgetItem *widgetItem = new QListWidgetItem(act.name);
            widgetItem->setToolTip(act.description);
            ui->act_list->addItem(widgetItem);
        }
    }

    ui->act_list->sortItems();
}

void MngActivity::load_activities()
{
    QSqlDatabase activityDb = QSqlDatabase::database("activity");

    QSqlQuery query(activityDb);

    if (!query.exec("SELECT id, name, description, duration, minPlayers, maxPlayers FROM Activities")) {
        qWarning() << "Query failed:" << query.lastError();
    } else {
        while (query.next()) {
            cur_act_list.insert(query.value(0).toInt(), {  query.value(1).toString(),
                                                            query.value(2).toString(),
                                                            query.value(3).toInt(),
                                                            query.value(4).toInt(),
                                                            query.value(5).toInt()});
        }
    }

    if (!query.exec("SELECT activityId, itemId FROM ActivityRequirements")) {
        qWarning() << "Query failed:" << query.lastError();
    } else {
        while (query.next()) {
            cur_act_list[query.value(0).toInt()].items.insert(query.value(1).toInt());
        }
    }

    if (!query.exec("SELECT activityId, tagId FROM ActivityTags")) {
        qWarning() << "Query failed:" << query.lastError();
    } else {
        while (query.next()) {
            cur_act_list[query.value(0).toInt()].tags.insert(query.value(1).toInt());
        }
    }

    new_act_list = cur_act_list;
}

void MngActivity::cancel_activity()
{
    ui->act_name_entry->clear();
    ui->act_desc_entry->clear();
    ui->act_dur_entry->setTime(QTime(0, 0));
    ui->act_min_p_spin->setValue(1);
    ui->act_max_p_spin->setValue(4);
    selected_items.clear();

    ui->confirm_act_btn->setText("Add activity");

    editing_act = -1;

    set_act_display();
    set_item_display();
}

void MngActivity::add_activity()
{
    const int id = editing_act;
    const QString name = ui->act_name_entry->text();
    const QString desc = ui->act_desc_entry->document()->toPlainText();
    const int dur = ui->act_dur_entry->time().hour()*60 + ui->act_dur_entry->time().minute();
    const int min = ui->act_min_p_slider->value();
    const int max = ui->act_max_p_spin->value();
    const QSet<int> reqs = selected_items;

    if (id>0)
    {
        auto act = new_act_list.find(id);
        if (act == new_act_list.end())
            return;

        QListWidgetItem *list_act = ui->act_list->findItems(act->name, Qt::MatchExactly)[0];

        act->name = name;
        act->description = desc;
        act->duration = dur;
        act->min_players = min;
        act->max_players = max;
        act->items = reqs;

        if (list_act)
        {
            list_act->setText(name);
            list_act->setToolTip(desc);
        }
    } else {
        if (find_same_name(name, new_act_list) != -1){
            QMessageBox::warning(
                this,
                tr("Simealplified"),
                tr("Unable to add activity. Another activity with that name already exists.") );
            return;
        }

        Activity act(name, desc, dur, min, max, reqs);


        const int new_id = generate_id(cur_act_list.keys(), new_act_list.keys());
        new_act_list.insert(new_id, act);

        auto *list_act = new QListWidgetItem(name);
        list_act->setToolTip(desc);
        ui->act_list->addItem(list_act);
    }

    ui->act_list->sortItems();

    cancel_activity();
}

void MngActivity::delete_activity()
{
    if (new_act_list.remove(find_same_name(ui->act_list->currentItem()->text(), new_act_list))) {
        ui->act_list->takeItem(ui->act_list->currentRow());
    } else {
        qWarning() << "Couldn't remove item";
    }

    cancel_activity();
}

void MngActivity::edit_activity(QListWidgetItem *list_item)
{
    int act_id = find_same_name(list_item->text(), new_act_list);

    if (act_id < 0) return;
    ui->confirm_act_btn->setText("Edit activity");
    auto activity = new_act_list[act_id];

    ui->act_name_entry->setText(activity.name);
    ui->act_desc_entry->setText(activity.description);
    ui->act_dur_entry->setTime(QTime(activity.duration/60, activity.duration%60));
    ui->act_min_p_spin->setValue(activity.min_players);
    ui->act_max_p_spin->setValue(activity.max_players);

    selected_items = activity.items;

    editing_act = act_id;

    set_item_display();
}
