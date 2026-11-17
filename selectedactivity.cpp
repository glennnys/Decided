#include "selectedactivity.h"
#include "ui_selectedactivity.h"
#include <QPropertyAnimation>
#include <QListWidget>

SelectedActivity::SelectedActivity(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SelectedActivity)
{
    ui->setupUi(this);

    connect(ui->revealBtn, &QPushButton::clicked, this, &SelectedActivity::toggleWidget);
    connect(ui->rerollBtn, &QPushButton::clicked, this, [this] {emit rerollActivity(selectedActivity);});

    ui->closeableWidget->setMaximumHeight(0);
}

SelectedActivity::~SelectedActivity()
{
    delete ui;
}

void SelectedActivity::loadData()
{
    QSqlDatabase activityDb = QSqlDatabase::database("activity");

    QSqlQuery query(activityDb);

    if (!query.exec("SELECT id, name FROM Tags")) {
        qWarning() << "Query failed:" << query.lastError();
    } else {
        while (query.next()) {
            tags.insert(query.value(0).toInt(), {query.value(1).toString()});
        }
    }

    if (!query.exec("SELECT id, name, description FROM Items")) {
        qWarning() << "Query failed:" << query.lastError();
    } else {
        while (query.next()) {
            items.insert(query.value(0).toInt(), {query.value(1).toString(), query.value(2).toString()});
        }
    }
}

void SelectedActivity::toggleWidget()
{
    //TODO: include window height
    bool opening =  ui->closeableWidget->maximumHeight() == 0;
    const int start = ui->closeableWidget->height();
    const int end = this->height() * opening;

    auto *animation = new QPropertyAnimation(ui->closeableWidget, "maximumHeight", this); //setting maximum height allows the widget to stretch and adapt normally instead of forcing height

    animation->setDuration(250);
    animation->setStartValue(start);
    animation->setEndValue(end);
    animation->setEasingCurve(QEasingCurve::OutCubic);

    animation->start(QAbstractAnimation::DeleteWhenStopped);

    if (opening)
    {
        ui->revealBtn->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoUp));
        ui->revealBtn->setText("Hide  ");
        ui->closeableWidget->setMaximumHeight(100000);
    }
    else
    {
        ui->revealBtn->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoDown));
        ui->revealBtn->setText("Reveal  ");
        ui->closeableWidget->setMaximumHeight(0);
    }
}

void SelectedActivity::fillActivity(Activity *selectedActivity)
{
    this->selectedActivity = selectedActivity;
    ui->namePlaceholder->setText(selectedActivity->name + "!");
    //ui->peoplePlaceholder->setText("Number of people: " + selectedActivity.min_players + "-" + selectedActivity.max_players);
    //ui->durationPlaceholder->setText("Estimated duration: " + str(selectedActivity.duration/60) + ":" + selectedActivity.duration%60);
    ui->descriptionBox->setText(selectedActivity->description);

    ui->tagList->clear();

    for (auto id : selectedActivity->tags)
    {
        QListWidgetItem *widgetItem = new QListWidgetItem(tags.find(id).value());
        ui->tagList->addItem(widgetItem);
    }

    ui->tagList->sortItems();

    ui->itemList->clear();

    for (auto id : selectedActivity->items)
    {
        QListWidgetItem *widgetItem = new QListWidgetItem(items.find(id).value().name);
            widgetItem->setToolTip(items.find(id).value().description);
            ui->itemList->addItem(widgetItem);
    }

    ui->itemList->sortItems();
}
