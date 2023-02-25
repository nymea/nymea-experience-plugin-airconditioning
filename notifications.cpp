#include "notifications.h"

#include <QUrlQuery>

Notifications::Notifications(ThingManager *thingManager, Thing *thing, QObject *parent)
    : QObject{parent},
      m_thingManager(thingManager),
      m_thing(thing)
{
    m_clearTimer.setInterval(30*60*1000);
    m_clearTimer.setSingleShot(true);
    connect(&m_clearTimer, &QTimer::timeout, this, [this](){
        m_isShown = false;
    });
}

void Notifications::update(const ZoneInfo &zone)
{
    bool supportsUpdate = m_thing->thingClassId() == ThingClassId("f0dd4c03-0aca-42cc-8f34-9902457b05de")
            && m_thing->paramValue("service").toString() == "FB-GCM"
            // Updating is only supported with versions that have the notificationId param
            && !m_thing->thingClass().actionTypes().findByName("notify").paramTypes().findByName("notificationId").id().isNull();

    QString notificationId = "humidityalert-" + zone.id().toString();
    QString title = "High humidity alert";
    QString text = QString("Humidity in zone %1: %2 %").arg(zone.name()).arg(zone.humidity());
    if (zone.zoneStatus().testFlag(ZoneInfo::ZoneStatusFlagHighHumidity)) {
        if (!m_isShown) {
            // show
            updateNotification(notificationId, title, text, false, false);
        } else if (supportsUpdate) {
            // update
            updateNotification(notificationId, title, text, false, false);
        }
    } else {
        if (m_isShown && supportsUpdate) {
            // remove
            updateNotification(notificationId, title, text, false, true);
        }
    }

    notificationId = "airalert-" + zone.id().toString();
    title = "Bad air alert";
    text = QString("Bad air in zone %1: %2").arg(zone.name());
    QStringList airValues;
    if (zone.voc() >= 660) {
        airValues.append(QString("%1 ppm").arg(zone.voc()));
    }
    if (zone.pm25() >= 25) {
        airValues.append(QString("%1 µg/m³").arg(zone.pm25()));
    }
    text = text.arg(airValues.join(","));
    if (zone.zoneStatus().testFlag(ZoneInfo::ZoneStatusFlagBadAir)) {
        if (!m_isShown) {
            // show
            updateNotification(notificationId, title, text, false, false);
        } else if (supportsUpdate) {
            // update
            updateNotification(notificationId, title, text, false, false);
        }
    } else {
        if (m_isShown && supportsUpdate) {
            // remove
            updateNotification(notificationId, title, text, false, true);
        }
    }
}

void Notifications::updateNotification(const QString &id, const QString &title, const QString &text, bool sound, bool remove)
{
    ActionType actionType = m_thing->thingClass().actionTypes().findByName("notify");
    Action action(actionType.id(), m_thing->id(), Action::TriggeredByRule);

    ParamList params = ParamList{
         Param(actionType.paramTypes().findByName("title").id(), title),
         Param(actionType.paramTypes().findByName("body").id(), text),
     };

    if (m_thing->thingClassId() == ThingClassId("f0dd4c03-0aca-42cc-8f34-9902457b05de")) {
        QUrlQuery data;
        data.addQueryItem("open", "airconditioning");
        params.append(Param(actionType.paramTypes().findByName("data").id(), data.toString()));

        // For backwards compatibility, only add those if the plugin already has them
        if (!m_thing->thingClass().actionTypes().findByName("notify").paramTypes().findByName("notificationId").id().isNull()) {
            params.append(Param(actionType.paramTypes().findByName("notificationId").id(), id));
            params.append(Param(actionType.paramTypes().findByName("sound").id(), sound));
            params.append(Param(actionType.paramTypes().findByName("remove").id(), remove));
        }
    }
    action.setParams(params);

    ThingActionInfo *info = m_thingManager->executeAction(action);
    connect(info, &ThingActionInfo::finished, this, [=](){
        if (info->status() == Thing::ThingErrorNoError) {
            m_isShown = !remove;
            m_clearTimer.start();
        }
    });

}
