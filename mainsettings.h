#ifndef MAINSETTINGS_H
#define MAINSETTINGS_H

#include <QWidget>

namespace Ui {
class MainSettings;
}

class MainSettings : public QWidget
{
    Q_OBJECT

public:
    explicit MainSettings(QWidget *parent = nullptr);
    ~MainSettings();

    void open();
    void close();
    void toggle();

private:
    Ui::MainSettings *ui;
    bool m_open = false;
};

#endif // MAINSETTINGS_H
