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

int generate_id(const QList<int>& ids)
{
    if (ids.isEmpty())
        return 1;

    return *std::max_element(ids.begin(), ids.end()) + 1;
}