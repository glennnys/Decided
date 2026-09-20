#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "activitypage.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QScrollArea *activity_scroll = new QScrollArea(this);
    ActivityPage *activity = new ActivityPage(this);
    activity_scroll->setWidget(activity);
    activity_scroll->setWidgetResizable(true);
    ui->stacked_pages->addWidget(activity_scroll);

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
