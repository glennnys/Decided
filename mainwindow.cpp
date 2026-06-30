#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mealpage.h"
#include "weekpage.h"
#include "activitypage.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QScrollArea *single_scroll = new QScrollArea(this);
    MealPage *single_meal = new MealPage(this);
    single_scroll->setWidget(single_meal);
    single_scroll->setWidgetResizable(true);
    ui->stacked_pages->addWidget(single_scroll);
    connect(ui->btn_page_single, &QPushButton::clicked, this, [this]{ changePage(0); });

    QScrollArea *week_scroll = new QScrollArea(this);
    WeekPage *week_meals = new WeekPage(this);
    week_scroll->setWidget(week_meals);
    week_scroll->setWidgetResizable(true);
    ui->stacked_pages->addWidget(week_scroll);
    connect(ui->btn_page_week, &QPushButton::clicked, this, [this]{ changePage(1); });

    QScrollArea *activity_scroll = new QScrollArea(this);
    ActivityPage *activity = new ActivityPage(this);
    activity_scroll->setWidget(activity);
    activity_scroll->setWidgetResizable(true);
    ui->stacked_pages->addWidget(activity_scroll);
    connect(ui->btn_page_activity, &QPushButton::clicked, this, [this]{ changePage(2); });

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changePage(int page)
{
    if (ui->stacked_pages->currentIndex() != page)
    {
        ui->stacked_pages->setCurrentIndex(page);
    }
}
