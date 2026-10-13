#ifndef MNGACTIVITY_H
#define MNGACTIVITY_H

#include <QWidget>
#include <QHash>
#include "structs.h"

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

    void clear();
    void load_items();
};

#endif // MNGACTIVITY_H
