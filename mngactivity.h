#ifndef MNGACTIVITY_H
#define MNGACTIVITY_H

#include <QWidget>
#include <QHash>
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

    void cancel_changes();
    void save_changes();

    void add_item();
    void load_items();
    void set_item_display();
    void cancel_item();
};

#endif // MNGACTIVITY_H
