#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "activitypage.h"
#include "mainsettings.h"
#include "mealpage.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Create settings menu
    MainSettings *settings = new MainSettings(this);

    //Hide settings menu
    settings->setGeometry(-250, 0, 250, height());
    settings->raise();

    //Burger menu toggles menu, Escape closes menu
    connect(ui->main_burger, &QPushButton::clicked, settings, &MainSettings::toggle);

    QShortcut *close_menu = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(close_menu, &QShortcut::activated, settings, &MainSettings::close);

    //Set selected page
    connect(settings, &MainSettings::page_selected, this, &MainWindow::changePage);


    //Scrollable area for activity page
    QScrollArea *activity_scroll = new QScrollArea(this);
    ActivityPage *activity = new ActivityPage(this);
    activity_scroll->setWidget(activity);
    activity_scroll->setWidgetResizable(true);
    ui->pages->addWidget(activity_scroll);

    //Scrollable area for meal page
    QScrollArea *meal_scroll = new QScrollArea(this);
    MealPage *meal = new MealPage(this);
    meal_scroll->setWidget(meal);
    meal_scroll->setWidgetResizable(true);
    ui->pages->addWidget(meal_scroll);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changePage(int page)
{
    if (ui->pages->currentIndex() != page)
    {
        ui->pages->setCurrentIndex(page);
    }
}
