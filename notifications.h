#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <QObject>
#include <QTimer>

#include <integrations/thing.h>
#include <integrations/thingmanager.h>

#include "zoneinfo.h"


class Notifications : public QObject
{
    Q_OBJECT
public:
    explicit Notifications(ThingManager *thingManager, Thing *thing, QObject *parent = nullptr);

    void update(const ZoneInfo &zone);
signals:

private:
    void updateNotification(const QString &id, const QString &title, const QString &text, bool sound, bool remove);
private:
    ThingManager *m_thingManager = nullptr;
    Thing *m_thing = nullptr;

    ZoneInfo::ZoneStatus m_zoneStatus;

    bool m_isShown = false;

    // For devices that don't support updates/removals, we'll assume after some time that it's gone and we may need to show again
    QTimer m_clearTimer;
};


#endif // NOTIFICATIONS_H
