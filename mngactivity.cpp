#include "mngactivity.h"
#include "ui_mngactivity.h"
#include "tag.h"
#include <QPropertyAnimation>
#include <QShortcut>


MngActivity::MngActivity(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MngActivity)
{
    ui->setupUi(this);

    auto shortcut = new QShortcut(QKeySequence(tr("Ctrl+S")), parent);
    connect(ui->save_btn, &QPushButton::clicked, this, &MngActivity::save_changes);
    connect(shortcut, &QShortcut::activated, this, &MngActivity::save_changes);
    connect(ui->cancel_btn, &QPushButton::clicked, this, &MngActivity::cancel_changes);

    itemWidget = new MngItems;
    tagWidget = new MngTags;
    ui->placeholder->addWidget(itemWidget);
    ui->placeholder->addWidget(tagWidget);
    connect(itemWidget, &MngItems::items_selected, this, [this] {setContentWidget(0);});
    connect(tagWidget, &MngTags::tags_selected, this, &MngActivity::set_tags);

    connect(ui->edit_item_btn, &QPushButton::clicked, this, [this] {setContentWidget(0);});
    connect(ui->edit_tags_btn, &QPushButton::clicked, this, [this] {setContentWidget(1);});

    flowLayout = new FlowLayout;
    ui->tag_holder->setLayout(flowLayout);

    editing_act = -1;
    connect(ui->search_act_entry, &QLineEdit::textChanged, this, &MngActivity::set_act_display);
    connect(ui->search_act_refresh, &QPushButton::clicked, this, [this] {ui->search_act_entry->clear();});
    connect(ui->confirm_act_btn, &QPushButton::clicked, this, &MngActivity::add_activity);
    connect(ui->cancel_act_btn, &QPushButton::clicked, this, &MngActivity::cancel_activity);
    connect(ui->delete_act_btn, &QPushButton::clicked, this, &MngActivity::delete_activity);
    connect(ui->act_list, &QListWidget::itemClicked, this, &MngActivity::edit_activity);

    connect(ui->act_min_p_spin, &QSpinBox::valueChanged, ui->act_max_p_spin, &QSpinBox::setMinimum);
    connect(ui->act_min_p_spin, &QSpinBox::valueChanged, ui->act_max_p_slider, &QSlider::setMinimum);

    load_activities();
    set_act_display();
}

MngActivity::~MngActivity()
{
    delete ui;
}

void MngActivity::initialize()
{
    emit finalizeActivities(cur_act_list, itemWidget->cur_item_list, tagWidget->cur_tag_list);
}

void MngActivity::setContentWidget(int page)
{
    auto *stack = ui->placeholder;
    if (page > stack->count()) return;

    const int current_page = stack->currentIndex();

    stack->setCurrentIndex(page);

    bool opening = stack->width() == 0 || current_page != page;
    change_editor(opening);
}

void MngActivity::change_editor(bool opening)
{
    if (editor_open == opening)
        return;

    const int editorWidth = 1000;
    const int start = ui->placeholder->width() * !opening;
    const int end = editorWidth * opening;

    auto *animation = new QPropertyAnimation(ui->placeholder, "maximumWidth", this);

    animation->setDuration(250);
    animation->setStartValue(start);
    animation->setEndValue(end);
    animation->setEasingCurve(QEasingCurve::OutCubic);

    animation->start(QAbstractAnimation::DeleteWhenStopped);

    ui->editWindowLayout->layout()->setSpacing(6*opening);

    editor_open = opening;
}

//General section
void MngActivity::cancel_changes()
{
    new_act_list = cur_act_list;

    itemWidget->cancel_changes();
    tagWidget->cancel_changes();

    set_act_display();

    flowLayout->clear();
}

void MngActivity::save_changes()
{
    itemWidget->save_changes();
    tagWidget->save_changes();

    QSqlDatabase activityDb = QSqlDatabase::database("activity");
    QSqlQuery query(activityDb);

    if (!query.exec("PRAGMA foreign_keys = ON")) {
        qWarning() << "Failed to enable foreign keys:"
                   << query.lastError();
    }

    //Items
    QList<int> toAdd;     // items to insert in DB
    QList<int> toRemove;  // items to delete from DB

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

    query.prepare(
        "INSERT INTO Activities "
        "(id, name, description, duration, minPlayers, MaxPlayers) "
        "VALUES (:id, :name, :desc, :dur, :min, :max) "
        "ON CONFLICT(id) DO UPDATE SET "
        "name = excluded.name, "
        "description = excluded.description, "
        "duration = excluded.duration, "
        "minPlayers = excluded.minPlayers, "
        "MaxPlayers = excluded.MaxPlayers"
        );

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


    //Activity item requirements
    QSqlQuery del_item_query(activityDb);
    QSqlQuery ins_item_query(activityDb);
    QSqlQuery del_tag_query(activityDb);
    QSqlQuery ins_tag_query(activityDb);

    del_item_query.prepare("DELETE FROM ActivityRequirements "
                  "WHERE activityId = :act_id AND itemId = :item_id ");

    ins_item_query.prepare("INSERT OR REPLACE INTO ActivityRequirements (activityId, itemId)"
                  "VALUES (:act_id, :item_id)");

    del_tag_query.prepare("DELETE FROM ActivityTags "
                      "WHERE activityId = :act_id AND tagId = :tag_id ");

    ins_tag_query.prepare("INSERT OR REPLACE INTO ActivityTags (activityId, tagId)"
                      "VALUES (:act_id, :tag_id)");

    for (const auto act_id : new_act_list.keys()) {
        const auto &new_act = new_act_list[act_id];

        QSet<int> itemsToAdd = new_act.items;
        QSet<int> tagsToAdd = new_act.tags;
        QSet<int> itemsToRemove;
        QSet<int> tagsToRemove;

        auto old_act = cur_act_list.constFind(act_id);

        if (old_act != cur_act_list.cend()) {
            itemsToRemove = old_act->items;
            itemsToRemove.subtract(new_act.items);
            itemsToAdd.subtract(old_act->items);

            tagsToRemove = old_act->tags;
            tagsToRemove.subtract(new_act.tags);
            tagsToAdd.subtract(old_act->tags);
        }


        for (const auto &item_id : std::as_const(itemsToRemove)) {
            del_item_query.bindValue(":act_id", act_id);
            del_item_query.bindValue(":item_id", item_id);

            if (!del_item_query.exec()) {
                qInfo() << "Deletion failed:" << del_item_query.lastError();
                activityDb.rollback();
                return;
            }
        }

        for (const auto &item_id : std::as_const(itemsToAdd)) {
            ins_item_query.bindValue(":act_id", act_id);
            ins_item_query.bindValue(":item_id", item_id);

            if (!ins_item_query.exec()) {
                qInfo() << "Insert failed:" << ins_item_query.lastError();
                activityDb.rollback();
                return;
            }
        }

        for (const auto &tag_id : std::as_const(tagsToRemove)) {
            del_tag_query.bindValue(":act_id", act_id);
            del_tag_query.bindValue(":tag_id", tag_id);

            if (!del_tag_query.exec()) {
                qInfo() << "Deletion failed:" << del_tag_query.lastError();
                activityDb.rollback();
                return;
            }
        }

        for (const auto &tag_id : std::as_const(tagsToAdd)) {
            ins_tag_query.bindValue(":act_id", act_id);
            ins_tag_query.bindValue(":tag_id", tag_id);

            if (!ins_tag_query.exec()) {
                qInfo() << "Insert failed:" << ins_tag_query.lastError();
                activityDb.rollback();
                return;
            }
        }
    }


    activityDb.commit(); // commit all inserts at once

    cur_act_list = new_act_list;

    QMessageBox msgBox(this);
    msgBox.setText("Changes have been saved.");
    msgBox.exec();

    emit finalizeActivities(cur_act_list, itemWidget->cur_item_list, tagWidget->cur_tag_list);
}

void MngActivity::set_tags(QHash<int, QString> tags)
{
    flowLayout->clear();

    for (auto [id, name] : tags.asKeyValueRange())
    {
        Tag *tag_widget = new Tag(this, name, id);
        flowLayout->addWidget(tag_widget);
        connect(tag_widget, &Tag::tag_removed, this, [this, tag_widget] {
            flowLayout->removeWidget(tag_widget);
            tagWidget->removeTag(tag_widget->tagId);
            tag_widget->hide();
            tag_widget->deleteLater();

            flowLayout->invalidate();
            flowLayout->activate();
        });
    }
}


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
                                                            query.value(5).toInt(),
                                                            {},
                                                            {} });
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
    itemWidget->selected_items.clear();

    ui->confirm_act_btn->setText("Add activity");

    editing_act = -1;

    itemWidget->cancel_item();
    tagWidget->selectedTags.clear();
    tagWidget->cancel_tag();
    flowLayout->clear();
    set_act_display();
}

void MngActivity::add_activity()
{
    const int id = editing_act;
    const QString name = ui->act_name_entry->text();
    const QString desc = ui->act_desc_entry->document()->toPlainText();
    const int dur = ui->act_dur_entry->time().hour()*60 + ui->act_dur_entry->time().minute();
    const int min = ui->act_min_p_slider->value();
    const int max = ui->act_max_p_spin->value();
    const QSet<int> reqs = itemWidget->selected_items;
    const QSet<int> tags = flowLayout->getTags();

    if (name == ""){
        QMessageBox::warning(
            this,
            tr("Decided"),
            tr("Write \"Doing nothing\" instead of leaving the activity name blank next time.") );
        return;
    }

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
        act->tags = tags;

        if (list_act)
        {
            list_act->setText(name);
            list_act->setToolTip(desc);
        }
    } else {
        if (find_same_name(name, new_act_list) != -1){
            QString response = "Unable to add activity. This town ain't big enough for two " + name + "'s.";

            QMessageBox::warning(
                this,
                tr("Decided"),
                tr(response.toLocal8Bit().data()) );
            return;
        }

        Activity act(name, desc, dur, min, max, reqs, tags);


        const int new_id = generate_id(cur_act_list.keys(), new_act_list.keys());
        new_act_list.insert(new_id, act);

        auto *list_act = new QListWidgetItem(name);
        list_act->setToolTip(desc);
        ui->act_list->addItem(list_act);
    }

    ui->act_list->sortItems();

    itemWidget->cancel_item();
    tagWidget->cancel_tag();
    cancel_activity();
    flowLayout->clear();
}

void MngActivity::delete_activity()
{
    auto item = ui->act_list->currentItem();
    if (!item) return;

    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Are you sure you want to delete \"" + item->text() + "\"?");
    msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();

    switch (ret) {
    case QMessageBox::Ok:
        if (new_act_list.remove(find_same_name(item->text(), new_act_list))) {
            ui->act_list->takeItem(ui->act_list->currentRow());
        } else {
            qWarning() << "Couldn't remove item";
        }

        itemWidget->cancel_item();
        tagWidget->cancel_tag();
        cancel_activity();
        return;
    case QMessageBox::Cancel:
        return;
    default:
        break;
    }
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

    itemWidget->selected_items = activity.items;
    itemWidget->initial_items = activity.items;
    tagWidget->selectedTags = activity.tags;
    tagWidget->initialTags = activity.tags;
    set_tags(tagWidget->getTags());

    editing_act = act_id;

    itemWidget->set_item_display();
    tagWidget->set_tag_display();
}
