#ifndef MNGITEMS_H
#define MNGITEMS_H

#include <QWidget>
#include <QHash>
#include <qlistwidget.h>
#include "utils.h"

namespace Ui {
class MngItems;
}

class MngItems : public QWidget
{
    Q_OBJECT

public:
    explicit MngItems(QWidget *parent = nullptr);
    ~MngItems();

    QHash<int, Item> cur_item_list, new_item_list;
    int editing_item;
    QSet<int> selected_items;

    void cancel_changes();
    void save_changes();

    void set_item_display();

    void cancel_item();

private:
    Ui::MngItems *ui;


    void load_items();
    void add_item();
    void delete_item();
    void edit_item(QListWidgetItem *item);

    void select_item();
    void deselect_item();
    void reset_selection();
};

#endif // MNGITEMS_H
