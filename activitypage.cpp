#include "activitypage.h"
#include "ui_activitypage.h"
#include "genactivity.h"
#include "mngactivity.h"

ActivityPage::ActivityPage(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ActivityPage)
{
    ui->setupUi(this);

    GenActivity *generator = new GenActivity(this);
    MngActivity *manager = new MngActivity(this);


    ui->tabs->addTab(generator, "Suggest activity");
    ui->tabs->addTab(manager, "Manage activities");

    connect(manager, &MngActivity::finalizeActivities, generator, &GenActivity::setData);

    manager->initialize();
}

ActivityPage::~ActivityPage()
{
    delete ui;
}
