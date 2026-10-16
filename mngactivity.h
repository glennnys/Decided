#ifndef MNGACTIVITY_H
#define MNGACTIVITY_H

#include <QWidget>
#include <QHash>
#include <qlistwidget.h>
#include "utils.h"
#include "mngitems.h"

namespace Ui {
class MngActivity;
}

class MngActivity : public QWidget
{
    Q_OBJECT

public:
    explicit MngActivity(QWidget *parent = nullptr);
    ~MngActivity();

private:
    Ui::MngActivity *ui;
    MngItems *itemWidget;

    QWidget *current_widget = nullptr;
    void setContentWidget(QWidget *widget);

    QHash<int, Activity> cur_act_list, new_act_list;
    int editing_act;

    void cancel_changes();
    void save_changes();

    void set_act_display();

    void load_activities();
    void cancel_activity();
    void add_activity();
    void delete_activity();
    void edit_activity(QListWidgetItem *item);

signals:
    void cancelled_changes();
    void cancelled_activity();

};

#endif // MNGACTIVITY_H
