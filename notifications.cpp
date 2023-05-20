#include "notifications.h"

#include <QUrlQuery>

Notifications::Notifications(ThingManager *thingManager, Thing *thing, QObject *parent)
    : QObject{parent},
      m_thingManager(thingManager),
      m_thing(thing)
{
    m_clearHumidityTimer.setInterval(30*60*1000);
    m_clearHumidityTimer.setSingleShot(true);
    connect(&m_clearHumidityTimer, &QTimer::timeout, this, [this](){
        m_humidityWarningShown = false;
    });

    m_clearBadAirTimer.setInterval(30*60*1000);
    m_clearBadAirTimer.setSingleShot(true);
    connect(&m_clearBadAirTimer, &QTimer::timeout, this, [this](){
        m_badAirWarningShown = false;
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
    ThingActionInfo *actionInfo = nullptr;
    if (zone.zoneStatus().testFlag(ZoneInfo::ZoneStatusFlagHighHumidity)) {
        if (!m_humidityWarningShown) {
            // show
            actionInfo = updateNotification(notificationId, title, text, false, false);
        } else if (supportsUpdate && !qFuzzyCompare(zone.humidity(), m_lastHumidityValue)) {
            // update
            actionInfo = updateNotification(notificationId, title, text, false, false);
        }
    } else {
        if (m_humidityWarningShown && supportsUpdate) {
            // remove
            actionInfo = updateNotification(notificationId, title, text, false, true);
        }
    }
    if (actionInfo) {
        connect(actionInfo, &ThingActionInfo::finished, this, [=](){
            if (actionInfo->status() == Thing::ThingErrorNoError) {
                m_humidityWarningShown = zone.zoneStatus().testFlag(ZoneInfo::ZoneStatusFlagHighHumidity);
                m_lastHumidityValue = zone.humidity();
                m_clearHumidityTimer.start();
            }
        });
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
    actionInfo = nullptr;
    if (zone.zoneStatus().testFlag(ZoneInfo::ZoneStatusFlagBadAir)) {
        if (!m_badAirWarningShown) {
            // show
            actionInfo = updateNotification(notificationId, title, text, false, false);
        } else if (supportsUpdate && zone.voc() != m_lastBadAirValue) {
            // update
            actionInfo = updateNotification(notificationId, title, text, false, false);
        }
    } else {
        if (m_badAirWarningShown && supportsUpdate) {
            // remove
            actionInfo = updateNotification(notificationId, title, text, false, true);
        }
    }
    if (actionInfo) {
        connect(actionInfo, &ThingActionInfo::finished, this, [=](){
            if (actionInfo->status() == Thing::ThingErrorNoError) {
                m_badAirWarningShown = zone.zoneStatus().testFlag(ZoneInfo::ZoneStatusFlagBadAir);
                m_lastBadAirValue = zone.voc();
                m_clearHumidityTimer.start();
            }
        });
    }
}

ThingActionInfo* Notifications::updateNotification(const QString &id, const QString &title, const QString &text, bool sound, bool remove)
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
    return info;
}
