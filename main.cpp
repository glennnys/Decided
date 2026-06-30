#include "mainwindow.h"

#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>

bool connectActivityDb(QString folder_path);


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);  // create folder if it doesn’t exist
    connectActivityDb(dataDir);


    MainWindow w;
    w.show();
    return a.exec();
}


bool connectActivityDb(QString folder_path){
    QString dbPath = folder_path + "/activities.db";
    bool isFirstRun = !QFile::exists(dbPath);

    QSqlDatabase activityDb = QSqlDatabase::addDatabase("QSQLITE", "activity");
    activityDb.setDatabaseName(dbPath);
    QSqlQuery query(activityDb);

    if (!activityDb.open()) {
        qWarning() << "Could not open database:" << activityDb.lastError();
        return false;
    }

    if (isFirstRun) {
        query.exec(R"( CREATE TABLE IF NOT EXISTS "Activities" (
            "id"	INTEGER NOT NULL UNIQUE,
            "name"	VARCHAR(255) NOT NULL,
            "duration"	INTEGER,
            "minPlayers"	INTEGER,
            "maxPlayers"	INTEGER,
            "explanation"	VARCHAR(255),
            "theme"	VARCHAR(255),
            PRIMARY KEY("id" AUTOINCREMENT),
            FOREIGN KEY("theme") REFERENCES "Themes"("name") ON UPDATE CASCADE ON DELETE SET NULL
        ); )");
        if (query.lastError().isValid()){ qDebug() << query.lastError(); }

        query.exec(R"( CREATE TABLE IF NOT EXISTS "ActivityRequirements" (
            "activityId"	INTEGER NOT NULL,
            "itemId"	INTEGER NOT NULL,
            CONSTRAINT "id" PRIMARY KEY("activityId","itemId"),
            FOREIGN KEY("activityId") REFERENCES "Activities"("id") ON UPDATE CASCADE ON DELETE CASCADE,
            FOREIGN KEY("itemId") REFERENCES "Items"("id") ON UPDATE CASCADE ON DELETE CASCADE
        ); )");
        if (query.lastError().isValid()){ qDebug() << query.lastError(); }

        query.exec(R"( CREATE TABLE IF NOT EXISTS "Items" (
                "id"	INTEGER NOT NULL UNIQUE,
                "name"	VARCHAR(255) NOT NULL UNIQUE,
                "description"	VARCHAR(255),
                PRIMARY KEY("id" AUTOINCREMENT)
            ); )");
        if (query.lastError().isValid()){ qDebug() << query.lastError(); }

        query.exec(R"( CREATE TABLE IF NOT EXISTS "Themes" (
            "name"	INTEGER NOT NULL UNIQUE,
            PRIMARY KEY("name")
        ); )");
        if (query.lastError().isValid()){ qDebug() << query.lastError(); }

    }

    return true;
}


