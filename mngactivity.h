#ifndef MNGACTIVITY_H
#define MNGACTIVITY_H

#include <QWidget>
#include <QHash>
#include <qlistwidget.h>
#include "utils.h"
#include "mngitems.h"
#include "mngtags.h"
#include "tag.h"

namespace Ui {
class MngActivity;
}

class MngActivity : public QWidget
{
    Q_OBJECT

public:
    explicit MngActivity(QWidget *parent = nullptr);
    ~MngActivity();

    void initialize();
private:
    Ui::MngActivity *ui;
    MngItems *itemWidget;
    MngTags *tagWidget;
    FlowLayout *flowLayout;

    QWidget *current_widget = nullptr;
    void setContentWidget(int page);
    bool editor_open = false;
    void change_editor(bool opening);

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

    void set_tags(QHash<int, QString> tags);

signals:
    void cancelled_changes();
    void cancelled_activity();

    void finalizeActivities(QHash<int, Activity> activities, QHash<int, Item> items, QHash<int, QString> tags);

};

#endif // MNGACTIVITY_H
