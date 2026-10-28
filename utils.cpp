#include <QLayout>
#include "utils.h"

template <typename T>
int find_same_name(const QString& name, const QHash<int, T>& list)
{
    for (auto it = list.cbegin(); it != list.cend(); ++it) {
        if (it.value().name.compare(name, Qt::CaseInsensitive) == 0) {
            return it.key();
        }
    }

    return -1;
}

// Explicit instantiations
template int find_same_name<Item>(const QString&, const QHash<int, Item>&);
template int find_same_name<Activity>(const QString&, const QHash<int, Activity>&);

int generate_id(const QList<int>& old_ids, const QList<int>& new_ids)
{
    if (old_ids.isEmpty() && new_ids.isEmpty())
        return 1;

    QSet<int> all_ids = QSet<int>(old_ids.begin(), old_ids.end());

    all_ids.unite(QSet<int>(new_ids.begin(), new_ids.end()));

    return *std::max_element(all_ids.begin(), all_ids.end()) + 1;
}

void clearLayout(QLayout *layout)
{
    if (!layout)
        return;

    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            delete widget;
        } else if (QLayout *childLayout = item->layout()) {
            clearLayout(childLayout);
            delete childLayout;
        }

        delete item;
    }

    delete layout;
}