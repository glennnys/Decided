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
