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
    this->setMaximumHeight(ui->topLabel->height());

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

    const int windowDelta = ui->closeableWidget->sizeHint().height();

    auto *contentAnim = new QPropertyAnimation(ui->closeableWidget, "maximumHeight", this);
    contentAnim->setDuration(250);
    contentAnim->setEasingCurve(QEasingCurve::OutCubic);

    auto *windowAnim = new QPropertyAnimation(this, "maximumHeight", this); // animates window size
    windowAnim->setDuration(250);
    windowAnim->setEasingCurve(QEasingCurve::OutCubic);
    windowAnim->setStartValue(this->height());
    windowAnim->setEndValue(this->height() + (opening ? windowDelta : -windowDelta));

    if (opening)
    {
        ui->closeableWidget->setMaximumHeight(QWIDGETSIZE_MAX);
        contentAnim->setStartValue(0);
        contentAnim->setEndValue(ui->closeableWidget->sizeHint().height());
        ui->revealBtn->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoUp));
        ui->revealBtn->setText("Hide  ");
    }
    else
    {
        contentAnim->setStartValue(ui->closeableWidget->height());
        contentAnim->setEndValue(0);
        ui->revealBtn->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoDown));
        ui->revealBtn->setText("Reveal  ");
    }

    // make sure the layout doesn't clamp the window below the animation target
    // connect(windowAnim, &QPropertyAnimation::valueChanged, this, [this] {this->setMinimumHeight(v.toInt());});

    windowAnim->start(QAbstractAnimation::DeleteWhenStopped);
    contentAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void SelectedActivity::fillActivity(Activity selectedActivity)
{
    this->selectedActivity = selectedActivity;
    ui->namePlaceholder->setText(selectedActivity.name + "!");
    if (selectedActivity.min_players == selectedActivity.max_players){
        ui->peoplePlaceholder->setText("Number of people: " + QString::number(selectedActivity.min_players));
    }
    else
    {
        ui->peoplePlaceholder->setText("Number of people: " + QString::number(selectedActivity.min_players) + "-" + QString::number(selectedActivity.max_players));
    }
    ui->durationPlaceholder->setText("Estimated duration: " + QString::number(selectedActivity.duration / 60) + ":" + QString::number(selectedActivity.duration % 60));
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
