#ifndef MNGACTIVITYTAGS_H
#define MNGACTIVITYTAGS_H

#include <QWidget>

namespace Ui {
class MngActivityTags;
}

class MngActivityTags : public QWidget
{
    Q_OBJECT

public:
    explicit MngActivityTags(QWidget *parent = nullptr);
    ~MngActivityTags();

private:
    Ui::MngActivityTags *ui;
};

#endif // MNGACTIVITYTAGS_H
