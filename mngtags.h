#ifndef MNGTAGS_H
#define MNGTAGS_H

#include <QWidget>
#include <QLayout>
#include <QStyle>
#include <qlistwidget.h>


namespace Ui {
class MngTags;
}

class MngTags : public QWidget
{
    Q_OBJECT

public:
    explicit MngTags(QWidget *parent = nullptr, bool selectOnly = false);
    ~MngTags();

    QHash<int, QString> cur_tag_list, new_tag_list;
    QSet<int> initialTags, selectedTags;
    int editing_tag;
    bool editable;

    void cancel_changes();
    void save_changes();

    void set_tag_display();

    void cancel_tag();

    void setTags(QSet<int> tagIds);
    void removeTag(int id);
    QHash<int, QString> getTags();
private:
    Ui::MngTags *ui;

    void load_tags();
    void add_tag();
    void delete_tag(QListWidgetItem *item);
    void toggleDelete();
    void edit_tag(QListWidgetItem *item);
    void confirm_tags();
    void cancel_tags();
signals:
    void tags_selected(QHash<int, QString> tags);
    void cancelled();
};

class FlowLayout : public QLayout
{
public:
    explicit FlowLayout(QWidget *parent, int margin = -1, int hSpacing = -1, int vSpacing = -1);
    explicit FlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
    ~FlowLayout();

    void addItem(QLayoutItem *item) override;
    int horizontalSpacing() const;
    int verticalSpacing() const;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int) const override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QLayoutItem *takeAt(int index) override;
    int indexOf(const QLayoutItem *item) const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect &rect) override;
    QSize sizeHint() const override;

    void clear();
    QSet<int> getTags();
private:
    int doLayout(const QRect &rect, bool testOnly) const;
    int smartSpacing(QStyle::PixelMetric pm) const;

    QList<QLayoutItem *> itemList;
    int m_hSpace;
    int m_vSpace;

};

#endif // MNGTAGS_H
