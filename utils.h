#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QHash>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

struct Item {
    QString name;
    QString description;

    bool operator==(const Item&) const = default;
};

struct Activity{
    QString name;
    QString description;
    int duration;
    int min_players;
    int max_players;
    QSet<int> items;
    QSet<int> tags;

    bool operator==(const Activity&) const = default;
};

template <typename T>
int find_same_name(const QString& name, const QHash<int, T>& list);

int generate_id(const QList<int>& old_ids, const QList<int>& new_ids);

#endif // UTILS_H
