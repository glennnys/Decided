#ifndef MNGACTIVITY_H
#define MNGACTIVITY_H

#include <QWidget>
#include <QHash>
#include <qlistwidget.h>
#include "utils.h"

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
    QHash<int, Item> cur_item_list, new_item_list;
    QHash<int, Activity> cur_act_list, new_act_list;
    int editing_item, editing_act;

    void cancel_changes();
    void save_changes();

    void add_item();
    void load_items();
    void delete_item();
    void cancel_item();
    void edit_item(QListWidgetItem *item);

    void set_item_display();
};

#endif // MNGACTIVITY_H
