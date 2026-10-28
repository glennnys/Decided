#ifndef TAG_H
#define TAG_H

#include <QWidget>

namespace Ui {
class Tag;
}

class Tag : public QWidget
{
    Q_OBJECT

public:
    explicit Tag(QWidget *parent = nullptr, QString name = "", int id = 0);
    ~Tag();

    QString tagName;
    int tagId;

private:
    Ui::Tag *ui;

signals:
    void tag_removed(Tag *tag);

};
#endif // TAG_H
