#ifndef STRUCTS_H
#define STRUCTS_H

#include <QString>
#include <QHash>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

struct Item {
    bool selected;
    QString name;
    QString description;
};

struct Activity{
    bool selected;
    QString name;
    int duration;
    int min_players;
    int max_players;
    QString explanation;
    QString theme;
    QHash<int, Item> items;
};

#endif // STRUCTS_H
