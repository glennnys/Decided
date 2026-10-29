#ifndef GENACTIVITY_H
#define GENACTIVITY_H

#include <QWidget>
#include "mngtags.h"
#include "utils.h"

namespace Ui {
class GenActivity;
}

class GenActivity : public QWidget
{
    Q_OBJECT

public:
    explicit GenActivity(QWidget *parent = nullptr);
    ~GenActivity();
private:
    Ui::GenActivity *ui;
    FlowLayout *tagholder;

    QHash<int, Activity> activities;
    QHash<int, Item> items;
    QHash<int, QString> tags;
    QHash<int, Location> currentLocations, newLocations;
    int currentLocationId;

    QSet<int> selectedTags;
    QSet<int> forcedTags;
    QSet<int> selectedItems;

    void generate();
    void setTags(QHash<int, QString> tags);

    void setItemDisplay();
    void selectItem();
    void deselectItem();
    void resetSelection();

    void setForcedTag(int id, bool toggle);
    void loadLocations();
    void saveLocations();
    void saveLocationsLocal();
    void resetCurrentLocation(QString locationName);
    void setCurrentLocation(QString locationName);
    void updateLocCombo(QString newItem = "");
    void deleteLocation();

public slots:
    void setData(QHash<int, Activity> activities, QHash<int, Item> items, QHash<int, QString> tags);
};

#endif // GENACTIVITY_H
