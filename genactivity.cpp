#include "genactivity.h"
#include "ui_genactivity.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QRandomGenerator>
#include "mngtags.h"
#include "tag.h"

GenActivity::GenActivity(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GenActivity)
{
    ui->setupUi(this);

    tagholder = new FlowLayout;
    ui->tagPlaceholder->setLayout(tagholder);

    // Adjusted tag selector without editing
    auto *selectTags = new QDialog(this);
    selectTags->setWindowTitle("Select tags");
    auto *layout = new QVBoxLayout(selectTags);
    auto *tagSelector = new MngTags(selectTags, true);
    layout->addWidget(tagSelector);
    selectTags->resize(500, 400);

    connect(ui->addTagsBtn, &QPushButton::clicked, selectTags, &QDialog::exec);
    connect(tagSelector, &MngTags::tags_selected, this, &GenActivity::setTags);
    connect(tagSelector, &MngTags::tags_selected, selectTags, &QDialog::close);
    connect(ui->searchAllItems, &QLineEdit::textChanged, this, &GenActivity::setItemDisplay);
    connect(ui->resetAllSearch, &QPushButton::clicked, this, [this] {ui->searchAllItems->clear();});
    connect(ui->selectItemBtn, &QPushButton::clicked, this, &GenActivity::selectItem);
    connect(ui->deselectItemBtn, &QPushButton::clicked, this, &GenActivity::deselectItem);
    connect(ui->resetSelect, &QPushButton::clicked, this, &GenActivity::resetSelection);
    connect(ui->allItemList, &QListWidget::itemDoubleClicked, this, &GenActivity::selectItem);
    connect(ui->selectItemList, &QListWidget::itemDoubleClicked, this, &GenActivity::deselectItem);
    connect(ui->getActiveBtn, &QPushButton::clicked, this, &GenActivity::generate);

    connect(ui->locCombo, &QComboBox::currentTextChanged, this, &GenActivity::resetCurrentLocation);
    connect(ui->locCombo, &QComboBox::textActivated, this, &GenActivity::setCurrentLocation);
    connect(ui->saveLocBtn, &QPushButton::clicked, this, &GenActivity::saveLocationsLocal);
    connect(ui->deleteLoc, &QPushButton::clicked, this, &GenActivity::deleteLocation);

    currentLocationId = -1;

    loadLocations();
}

GenActivity::~GenActivity()
{
    delete ui;
}

void GenActivity::setData(QHash<int, Activity> activities, QHash<int, Item> items, QHash<int, QString> tags)
{
    this->activities = activities;
    this->items = items;
    this->tags = tags;

    setItemDisplay();
}

void GenActivity::generate()
{
    QHash<int, Activity> availableActivities = activities;
    QList<QPair<int, Activity>> possibleActivities;
    int totalWeight = 0;

    bool forceItems = ui->forceItemCheck->isChecked();
    int maxDuration = ui->durationSpin->time().hour()*60 + ui->durationSpin->time().minute();
    int personCount = ui->peopleCntSpin->value();

    for (const auto &[id, activity] : availableActivities.asKeyValueRange())
    {
        if (maxDuration != 0 && activity.duration > maxDuration) continue;
        if (activity.max_players < personCount) continue;
        if (personCount != 0 && activity.min_players > personCount) continue;

        QSet<int> nonAvailableItems = activity.items;
        QSet<int> unusedItems = selectedItems;
        nonAvailableItems.subtract(selectedItems);
        unusedItems.subtract(activity.items);

        if (activity.items.count() > 0 && nonAvailableItems.count() > 0) continue;
        if (forceItems && unusedItems.count() > 0) continue;

        QSet<int> overlapForcedTags = forcedTags;
        if (overlapForcedTags.subtract(activity.tags).count() != 0) continue;

        QSet<int> overlap = activity.tags;
        overlap.intersect(selectedTags);

        possibleActivities.append(qMakePair(overlap.count() + 1, activity));

        totalWeight += overlap.count() + 1;
    }

    if (possibleActivities.count() == 0)
    {
        QMessageBox msgBox(this);
        msgBox.setText("Nothing found...");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;
    }

    int rand = QRandomGenerator::global()->bounded(totalWeight);

    for (const auto &activity : possibleActivities)
    {
        if (rand < activity.first)
        {
            QMessageBox msgBox(this);
            msgBox.setText("Your activity is...");
            msgBox.setDetailedText(activity.second.name + "!");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.exec();

            return;
        } else {
            rand -= activity.first;
        }
    }
}

void GenActivity::loadLocations()
{
    QSqlDatabase activityDb = QSqlDatabase::database("activity");

    QSqlQuery query(activityDb);

    if (!query.exec("SELECT id, name FROM Locations")) {
        qWarning() << "Query failed:" << query.lastError();
    } else {
        while (query.next()) {
            currentLocations.insert(query.value(0).toInt(), {query.value(1).toString()});
        }
    }

    if (!query.exec("SELECT locationId, itemId FROM ItemsAtLocation")) {
        qWarning() << "Query failed:" << query.lastError();
    } else {
        while (query.next()) {
            currentLocations[query.value(0).toInt()].items.insert(query.value(1).toInt());
        }
    }

    newLocations = currentLocations;
    updateLocCombo();
}

void GenActivity::updateLocCombo(QString newItem)
{
    if (newItem == "")
    {
        ui->locCombo->clear();
        for (auto [id, location] : newLocations.asKeyValueRange())
        {
            ui->locCombo->addItem(location.name);
        }

        return;
    }

    ui->locCombo->addItem(newItem);
}

void GenActivity::resetCurrentLocation(QString locationName)
{
    if (locationName == "") {
        currentLocationId = -1;
    }
}

void GenActivity::setCurrentLocation(QString locationName)
{
    int newLocationId = find_same_name(locationName, newLocations);

    if (newLocationId > 0 && newLocationId != currentLocationId)
    {
        currentLocationId = newLocationId;
        selectedItems = newLocations[currentLocationId].items;
        setItemDisplay();
        return;
    }
}

void GenActivity::saveLocationsLocal()
{
    if (ui->locCombo->currentText() == "") return;

    if (currentLocationId > 0)
    {
        newLocations[currentLocationId].name = ui->locCombo->currentText();
        newLocations[currentLocationId].items = selectedItems;
    }
    else
    {
        auto name = ui->locCombo->currentText();
        newLocations.insert(generate_id(currentLocations.keys(), newLocations.keys()), {ui->locCombo->currentText(), selectedItems});
    }

    saveLocations();

    updateLocCombo();
}

void GenActivity::saveLocations()
{
    QSqlDatabase activityDb = QSqlDatabase::database("activity");
    QSqlQuery query(activityDb);

    if (!query.exec("PRAGMA foreign_keys = ON")) {
        qWarning() << "Failed to enable foreign keys:"
                   << query.lastError();
    }

    //Items
    QSet<int> toAdd;     // items to insert in DB
    QSet<int> toRemove;  // items to delete from DB

    //Activities
    toAdd.clear();
    toRemove.clear();

    // Detect additions and updates
    for (auto key : newLocations.keys()) {
        if (!currentLocations.contains(key)) {
            toAdd.insert(key); // new local item
        }
        else if (newLocations[key] != currentLocations[key]) {
            toAdd.insert(key); // edited local item
        }
    }

    // Detect removals
    for (auto key : currentLocations.keys()) {
        if (!newLocations.contains(key)) {
            toRemove.insert(key);
        }
    }

    activityDb.transaction();

    query.prepare("DELETE FROM Locations "
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
        "INSERT INTO Locations "
        "(id, name) "
        "VALUES (:id, :name) "
        "ON CONFLICT(id) DO UPDATE SET "
        "name = excluded.name "
        );

    for (const auto &id : std::as_const(toAdd)) {
        query.bindValue(":id", id);
        query.bindValue(":name", newLocations[id].name);

        if (!query.exec()) {
            qInfo() << "Insert failed:" << query.lastError();
            activityDb.rollback();
            return;
        }
    }

    //Activity item requirements
    QSqlQuery delQuery(activityDb);
    QSqlQuery insertQuery(activityDb);

    delQuery.prepare("DELETE FROM ItemsAtLocation "
                           "WHERE locationId = :locId AND itemId = :itemId ");

    insertQuery.prepare("INSERT OR REPLACE INTO ItemsAtLocation (locationId, itemId)"
                           "VALUES (:locId, :itemId)");

    for (const auto locId : newLocations.keys()) {
        const auto &newLoc = newLocations[locId];
            toAdd = newLoc.items;

        auto old_act = currentLocations.constFind(locId);

        if (old_act != currentLocations.cend()) {
            toRemove = old_act->items;
            toRemove.subtract(newLoc.items);
            toAdd.subtract(old_act->items);
        }

        for (const auto &itemId : std::as_const(toRemove)) {
            delQuery.bindValue(":locId", locId);
            delQuery.bindValue(":itemId", itemId);

            if (!delQuery.exec()) {
                qInfo() << "Deletion failed:" << delQuery.lastError();
                activityDb.rollback();
                return;
            }
        }

        for (const auto &itemId : std::as_const(toAdd)) {
            insertQuery.bindValue(":locId", locId);
            insertQuery.bindValue(":itemId", itemId);

            if (!insertQuery.exec()) {
                qInfo() << "Insert failed:" << insertQuery.lastError();
                activityDb.rollback();
                return;
            }
        }
    }


    activityDb.commit(); // commit all inserts at once

    currentLocations = newLocations;
    updateLocCombo();
}

void GenActivity::deleteLocation()
{
    if (currentLocationId < 0) return;

    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Are you sure you want to delete \"" + newLocations[currentLocationId].name + "\"?");
    msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();

    switch (ret) {
    case QMessageBox::Ok:
        if (newLocations.remove(currentLocationId)) {
            currentLocationId = -1;
            saveLocations();
            selectedItems.clear();
            setItemDisplay();
        } else {
            qWarning() << "Couldn't remove item";
        }

        return;
    case QMessageBox::Cancel:
        return;
    default:
        break;
    }
}

void GenActivity::setTags(QHash<int, QString> tags)
{
    tagholder->clear();
    selectedTags.clear();

    for (auto [id, name] : tags.asKeyValueRange())
    {
        selectedTags.insert(id);
        Tag *tag = new Tag(this, name, id, true);
        tagholder->addWidget(tag);
        connect(tag, &Tag::tag_removed, this, [this, tag] {
            tagholder->removeWidget(tag);
            selectedTags.remove(tag->tagId);

            tag->hide();
            tag->deleteLater();

            tagholder->invalidate();
            tagholder->activate();
        });

        connect(tag, &Tag::toggled, this, &GenActivity::setForcedTag);
    }
}

void GenActivity::setItemDisplay()
{
    ui->allItemList->clear();
    ui->selectItemList->clear();

    for (auto [id, item] : items.asKeyValueRange())
    {
        if (item.name.toLower().contains(ui->searchAllItems->text().toLower()) || item.description.toLower().contains(ui->searchAllItems->text().toLower()))
        {
            QListWidgetItem *widgetItem = new QListWidgetItem(item.name);
            widgetItem->setToolTip(item.description);
            if (selectedItems.contains(id)) ui->selectItemList->addItem(widgetItem);
            else ui->allItemList->addItem(widgetItem);
        }
    }

    ui->allItemList->sortItems();
    ui->selectItemList->sortItems();
}

void GenActivity::selectItem()
{
    if (!ui->allItemList->currentItem()) return;

    // remove item from unselected list view and list
    auto selected_item = ui->allItemList->takeItem(ui->allItemList->currentRow());
    int item_id = find_same_name(selected_item->text(), items);

    // add item to selected list view
    ui->selectItemList->addItem(selected_item);
    ui->selectItemList->sortItems();

    // add item to selected list
    selectedItems.insert(item_id);

    setItemDisplay();
}

void GenActivity::deselectItem()
{
    if (!ui->selectItemList->currentItem()) return;

    // remove item from selected list view and list
    auto deselected_item = ui->selectItemList->takeItem(ui->selectItemList->currentRow());
    int item_id = find_same_name(deselected_item->text(), items);

    // add item to existing list view
    ui->allItemList->addItem(deselected_item);
    ui->allItemList->sortItems();

    // remove item from selected list
    selectedItems.remove(item_id);

    setItemDisplay();
}

void GenActivity::resetSelection()
{
    ui->selectItemList->clear();

    selectedItems.clear();

    setItemDisplay();
}

void GenActivity::setForcedTag(int id, bool forced)
{

    if (forcedTags.contains(id)) {
        if (!forced) forcedTags.remove(id);
    } else {
        forcedTags.insert(id);
    }
}
