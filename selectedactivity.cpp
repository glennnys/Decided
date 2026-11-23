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
    connect(ui->closeBtn, &QPushButton::clicked, this, &SelectedActivity::hide);


    ui->closeableWidget->setMaximumHeight(0);
    this->setMaximumHeight(0);

    loadData();
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
    const bool opening = ui->closeableWidget->maximumHeight() == 0;

    if (opening)
    {
        ui->revealBtn->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoUp));
        ui->revealBtn->setText("Hide  ");

        int start = 0;
        int end = ui->closeableWidget->sizeHint().height();

        auto *animation = new QPropertyAnimation(ui->closeableWidget, "maximumHeight");
        animation->setDuration(250);
        animation->setStartValue(start);
        animation->setEndValue(end);
        animation->setEasingCurve(QEasingCurve::OutCubic);

        animation->start(QAbstractAnimation::DeleteWhenStopped);

        connect(animation, &QPropertyAnimation::finished, this, [this]() {
            ui->closeableWidget->setMaximumHeight(QWIDGETSIZE_MAX);
            this->setMaximumHeight(QWIDGETSIZE_MAX);
        });
    }
    else
    {
        this->setMaximumHeight(0);
        ui->revealBtn->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoDown));
        ui->revealBtn->setText("Reveal  ");

        int start = ui->closeableWidget->height();
        int end = 0;

        auto *animation = new QPropertyAnimation(ui->closeableWidget, "maximumHeight");
        animation->setDuration(250);
        animation->setStartValue(start);
        animation->setEndValue(end);
        animation->setEasingCurve(QEasingCurve::OutCubic);

        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void SelectedActivity::fillActivity(Activity selectedActivity)
{
    if (selectedActivity.name == "") return;

    if (this->isHidden())
    {
        this->show();
    }

    loadData();

    this->selectedActivity = selectedActivity;
    ui->namePlaceholder->setText(selectedActivity.name + "!");

    //player count formatting
    if (selectedActivity.min_players == selectedActivity.max_players){
        ui->peoplePlaceholder->setText("Number of players: " + QString::number(selectedActivity.min_players));
    }
    else
    {
        ui->peoplePlaceholder->setText("Number of players: " + QString::number(selectedActivity.min_players) + "-" + QString::number(selectedActivity.max_players));
    }

    //duration formatting
    if (selectedActivity.duration < 60)
    {
        ui->durationPlaceholder->setText("Estimated duration: " + QString::number(selectedActivity.duration) + "m");
    }
    else if (selectedActivity.duration/60 == 0)
    {
        ui->durationPlaceholder->setText("Estimated duration: " + QString::number(selectedActivity.duration/60) + "h");
    }
    else
    {
        ui->durationPlaceholder->setText("Estimated duration: " + QString::number(selectedActivity.duration / 60) + "h" + QString::number(selectedActivity.duration % 60) + "m");
    }

    ui->descriptionBox->setText(selectedActivity.description);

    ui->tagList->clear();

    for (auto id : selectedActivity.tags)
    {
        QListWidgetItem *widgetItem = new QListWidgetItem(tags.find(id).value());
        ui->tagList->addItem(widgetItem);
    }

    ui->tagList->sortItems();

    ui->itemList->clear();

    for (auto id : selectedActivity.items)
    {
        QListWidgetItem *widgetItem = new QListWidgetItem(items.find(id).value().name);
            widgetItem->setToolTip(items.find(id).value().description);
            ui->itemList->addItem(widgetItem);
    }

    ui->itemList->sortItems();
}
