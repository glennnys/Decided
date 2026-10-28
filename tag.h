#ifndef TAG_H
#define TAG_H

#include <QWidget>
#include <QCheckBox>

namespace Ui {
class Tag;
}

class Tag : public QWidget
{
    Q_OBJECT

public:
    explicit Tag(QWidget *parent = nullptr, QString name = "", int id = 0, bool showToggle = false);
    ~Tag();

    QString tagName;
    int tagId;

private:
    Ui::Tag *ui;
    QCheckBox *toggleBtn = nullptr;

signals:
    void tag_removed(Tag *tag);
    void toggled(int id, bool state);

};
#endif // TAG_H
