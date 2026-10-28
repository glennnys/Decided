#include "mngtags.h"
#include "ui_mngtags.h"
#include "utils.h"
#include "tag.h"

MngTags::MngTags(QWidget *parent, bool selectOnly)
    : QWidget(parent)
    , ui(new Ui::MngTags)
{
    ui->setupUi(this);

    editing_tag = -1;
    editable = !selectOnly;
    connect(ui->search_edit, &QLineEdit::textChanged, this, &MngTags::set_tag_display);
    connect(ui->add_tag_btn, &QPushButton::clicked, this, &MngTags::add_tag);
    connect(ui->deleteBtn, &QPushButton::clicked, this, &MngTags::toggleDelete);
    connect(ui->tag_entry, &QLineEdit::returnPressed, this, &MngTags::add_tag);
    connect(ui->resetTagBtn, &QPushButton::clicked, this, &MngTags::cancel_tag);
    connect(ui->confirmBtnBox, &QDialogButtonBox::accepted, this, &MngTags::confirm_tags);
    connect(ui->confirmBtnBox, &QDialogButtonBox::rejected, this, &MngTags::cancel_tags);

    ui->deleteBtn->setChecked(false);
    toggleDelete();
    load_tags();
    set_tag_display();

    if (selectOnly)
    {
        ui->deleteBtn->hide();
        ui->deleteBtn->deleteLater();
        clearLayout(ui->addTagLayout->layout());
    }
}

MngTags::~MngTags()
{
    delete ui;
}

void MngTags::cancel_changes()
{
    new_tag_list = cur_tag_list;

    set_tag_display();
}

void MngTags::save_changes()
{
    QSqlDatabase activityDb = QSqlDatabase::database("activity");
    QSqlQuery query(activityDb);

    if (!query.exec("PRAGMA foreign_keys = ON")) {
        qWarning() << "Failed to enable foreign keys:"
                   << query.lastError();
    }

    //Items
    QList<int> toAdd;     // items to insert in DB
    QList<int> toRemove;  // items to delete from DB

    // Detect additions and updates
    for (auto key : new_tag_list.keys()) {
        if (!cur_tag_list.contains(key)) {
            toAdd.append(key); // new local item
        }
        else if (new_tag_list[key] != cur_tag_list[key]) {
            toAdd.append(key); // edited local item
        }
    }

    // Detect removals
    for (auto key : cur_tag_list.keys()) {
        if (!new_tag_list.contains(key)) {
            toRemove.append(key);
        }
    }

    activityDb.transaction();

    query.prepare("DELETE FROM Tags "
                  "WHERE id = :id");

    for (const auto &id : std::as_const(toRemove)) {
        query.bindValue(":id", id);

        if (!query.exec()) {
            qInfo() << "Deletion failed:" << query.lastError();
            activityDb.rollback();
            return;
        }
    }

    query.prepare(
        "INSERT INTO Tags (id, name) "
        "VALUES (:id, :name) "
        "ON CONFLICT(id) DO UPDATE SET "
        "name = excluded.name"
        );

    for (const auto &id : std::as_const(toAdd)) {
        query.bindValue(":id", id);
        query.bindValue(":name", new_tag_list[id]);

        if (!query.exec()) {
            qInfo() << "Insert failed:" << query.lastError();
            activityDb.rollback();
            return;
        }
    }

    activityDb.commit(); // commit all inserts at once

    cur_tag_list = new_tag_list;
}

void MngTags::confirm_tags()
{
    QHash<int, QString> tags;
    for (auto tag : ui->tag_list->selectedItems())
    {
        const auto id = new_tag_list.key(tag->text());
        const auto value = new_tag_list.find(id);
        if (value == new_tag_list.end()) return;
        tags.insert(id, value.value());
    }

    emit tags_selected(tags);
}

void MngTags::cancel_tags()
{
    selectedTags = initialTags;
    set_tag_display();

    emit cancelled();
}

void MngTags::set_tag_display()
{
    ui->tag_list->clear();

    for (auto [id, tag] : new_tag_list.asKeyValueRange())
    {
        if (tag.toLower().contains(ui->search_edit->text().toLower()))
        {
            QListWidgetItem *widgetItem = new QListWidgetItem(tag);
            ui->tag_list->addItem(widgetItem);
            if (selectedTags.contains(id)) widgetItem->setSelected(true);
        }
    }

    ui->tag_list->sortItems();
}

void MngTags::load_tags()
{
    QSqlDatabase activityDb = QSqlDatabase::database("activity");

    QSqlQuery query(activityDb);

    if (!query.exec("SELECT id, name FROM Tags")) {
        qWarning() << "Query failed:" << query.lastError();
    } else {
        while (query.next()) {
            cur_tag_list.insert(query.value(0).toInt(), {query.value(1).toString()});
        }
    }

    new_tag_list = cur_tag_list;
}

void MngTags::cancel_tag()
{
    if (!editable) return;
    ui->tag_entry->clear();

    ui->tag_label->setText("New tag");
    ui->add_tag_btn->setText("Add");

    editing_tag = -1;

    set_tag_display();
}

void MngTags::add_tag()
{
    if (!editable) return;
    const int id = editing_tag;
    const QString name = ui->tag_entry->text();

    if (name == ""){
        QMessageBox::warning(
            this,
            tr("Decided"),
            tr("Or you could simply not use a tag instead?") );
        return;
    }

    if (id>0)
    {
        auto tag = new_tag_list.find(id);
        if (tag == new_tag_list.end())
            return;

        QListWidgetItem *list_tag = ui->tag_list->findItems(tag.value(), Qt::MatchExactly)[0] ;

        tag.value() = name;


        if (list_tag)
        {
            list_tag->setText(name);
        }

        ui->tag_list->sortItems();

    } else {
        if (new_tag_list.values().contains(name)){
            QMessageBox::warning(
                this,
                tr("Decided"),
                tr("I would allow two tags with the same name, but I don't see much point in it.") );
            return;
        }

        const int new_id = generate_id(cur_tag_list.keys(), new_tag_list.keys());
        new_tag_list.insert(new_id, name);

        auto *list_item = new QListWidgetItem(name);
        ui->tag_list->addItem(list_item);

        ui->tag_list->sortItems();
    }

    cancel_tag();

    ui->tag_entry->setFocus();
}

void MngTags::delete_tag(QListWidgetItem *item)
{
    if (!editable) return;
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Are you sure you want to delete the \"" + item->text() + "\" tag?");
    msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();

    switch (ret) {
    case QMessageBox::Save:
        if (new_tag_list.remove(new_tag_list.key(item->text()))) {
            ui->tag_list->removeItemWidget(item);
        } else {
            qWarning() << "Couldn't remove item";
        }

        cancel_tag();
        return;
    case QMessageBox::Cancel:
        return;
    default:
        break;
    }
}

void MngTags::toggleDelete()
{
    if (!editable) return;
    if (ui->deleteBtn->isChecked())
    {
        disconnect(ui->tag_list, &QListWidget::itemClicked, this, &MngTags::edit_tag);
        connect(ui->tag_list, &QListWidget::itemClicked, this, &MngTags::delete_tag);
        ui->deleteBtn->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::FormatJustifyFill));
        ui->tag_list->setSelectionMode(QAbstractItemView::NoSelection);
        return;
    } else {
        disconnect(ui->tag_list, &QListWidget::itemClicked, this, &MngTags::delete_tag);
        connect(ui->tag_list, &QListWidget::itemClicked, this, &MngTags::edit_tag);
        ui->deleteBtn->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::EditDelete));
        ui->tag_list->setSelectionMode(QAbstractItemView::MultiSelection);
        return;
    }
}

void MngTags::edit_tag(QListWidgetItem *list_item)
{
    if (!editable) return;
    int item_id = new_tag_list.key(list_item->text());

    if (item_id < 0) return;
    ui->tag_label->setText("Edit tag");
    ui->add_tag_btn->setText("Edit");

    ui->tag_entry->setText(list_item->text());

    editing_tag = item_id;
}

QHash<int, QString> MngTags::getTags(){
    QHash<int, QString> tags;
    for (auto id : selectedTags) {
        tags.insert(id, new_tag_list.find(id).value());
    }
    return tags;
}

void MngTags::removeTag(int id)
{
    selectedTags.remove(id);
    set_tag_display();
}

// Layout for adding tags somewhere
FlowLayout::FlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::FlowLayout(int margin, int hSpacing, int vSpacing)
    : m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::~FlowLayout()
{
    clear();
}

void FlowLayout::addItem(QLayoutItem *item)
{
    itemList.append(item);
}

void FlowLayout::clear()
{
    while (QLayoutItem *item = takeAt(0)) {
        if (QWidget *widget = item->widget())
            widget->deleteLater();

        delete item;
    }

    invalidate();
    activate();
}

QSet<int> FlowLayout::getTags()
{
    QSet<int> tags;
    for (auto item : itemList)
    {
        if (auto *tag = qobject_cast<Tag *>(item->widget())) {
            tags.insert(tag->tagId);
        }
    }

    return tags;
}

int FlowLayout::horizontalSpacing() const
{
    if (m_hSpace >= 0) {
        return m_hSpace;
    } else {
        return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
    }
}

int FlowLayout::verticalSpacing() const
{
    if (m_vSpace >= 0) {
        return m_vSpace;
    } else {
        return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
    }
}

int FlowLayout::count() const
{
    return itemList.size();
}

QLayoutItem *FlowLayout::itemAt(int index) const
{
    return itemList.value(index);
}

QLayoutItem *FlowLayout::takeAt(int index)
{
    if (index >= 0 && index < itemList.size())
        return itemList.takeAt(index);
    return nullptr;
}

int FlowLayout::indexOf(const QLayoutItem *item) const
{
    return itemList.indexOf(item);
}

Qt::Orientations FlowLayout::expandingDirections() const
{
    return { };
}

bool FlowLayout::hasHeightForWidth() const
{
    return true;
}

int FlowLayout::heightForWidth(int width) const
{
    int height = doLayout(QRect(0, 0, width, 0), true);
    return height;
}

void FlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

QSize FlowLayout::sizeHint() const
{
    return minimumSize();
}

QSize FlowLayout::minimumSize() const
{
    QSize size;
    for (const QLayoutItem *item : std::as_const(itemList))
        size = size.expandedTo(item->minimumSize());

    const QMargins margins = contentsMargins();
    size += QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
    return size;
}

int FlowLayout::doLayout(const QRect &rect, bool testOnly) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    for (QLayoutItem *item : std::as_const(itemList)) {
        const QWidget *wid = item->widget();
        int spaceX = horizontalSpacing();
        if (spaceX == -1)
            spaceX = wid->style()->layoutSpacing(
                QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
        int spaceY = verticalSpacing();
        if (spaceY == -1)
            spaceY = wid->style()->layoutSpacing(
                QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);

        int nextX = x + item->sizeHint().width() + spaceX;
        if (nextX - spaceX > effectiveRect.right() && lineHeight > 0) {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + item->sizeHint().width() + spaceX;
            lineHeight = 0;
        }

        if (!testOnly)
            item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
    return y + lineHeight - rect.y() + bottom;
}

int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const
{
    QObject *parent = this->parent();
    if (!parent) {
        return -1;
    } else if (parent->isWidgetType()) {
        QWidget *pw = static_cast<QWidget *>(parent);
        return pw->style()->pixelMetric(pm, nullptr, pw);
    } else {
        return static_cast<QLayout *>(parent)->spacing();
    }
}
