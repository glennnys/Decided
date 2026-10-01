#include "mainsettings.h"
#include "ui_mainsettings.h"

#include <QPropertyAnimation>

MainSettings::MainSettings(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainSettings)
{
    ui->setupUi(this);

    connect(ui->close, &QPushButton::clicked, this, &MainSettings::close);
}

MainSettings::~MainSettings()
{
    delete ui;
}

void MainSettings::open()
{
    if (m_open)
        return;

    int width = this->width();

    QRect start(-width, 0, width, parentWidget()->height());
    QRect end(0, 0, width, parentWidget()->height());

    auto *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(250);
    animation->setStartValue(start);
    animation->setEndValue(end);
    animation->setEasingCurve(QEasingCurve::OutCubic);

    raise();
    show();

    animation->start(QAbstractAnimation::DeleteWhenStopped);

    m_open = true;
}

void MainSettings::close()
{
    if (!m_open)
        return;

    int width = this->width();

    QRect start(0, 0, width, parentWidget()->height());
    QRect end(-width, 0, width, parentWidget()->height());

    auto *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(250);
    animation->setStartValue(start);
    animation->setEndValue(end);
    animation->setEasingCurve(QEasingCurve::InCubic);

    animation->start(QAbstractAnimation::DeleteWhenStopped);

    m_open = false;
}

void MainSettings::toggle()
{
    if (m_open)
        close();
    else
        open();
}
