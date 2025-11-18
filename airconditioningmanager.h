// SPDX-License-Identifier: GPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea-experience-plugin-airconditioning.
*
* nymea-experience-plugin-airconditioning is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* nymea-experience-plugin-airconditioning is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nymea-experience-plugin-airconditioning. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef AIRCONDITIONINGMANAGER_H
#define AIRCONDITIONINGMANAGER_H

#include <QObject>
#include <QHash>
#include <QTimer>

#include <integrations/thingmanager.h>

#include "zoneinfo.h"
#include "thermostat.h"
#include "notifications.h"

class AirConditioningManager : public QObject
{
    Q_OBJECT
public:
    enum AirConditioningError {
        AirConditioningErrorNoError,
        AirConditioningErrorZoneNotFound,
        AirConditioningErrorInvalidTimeSpec,
        AirConditioningErrorThingNotFound,
        AirConditioningErrorInvalidThingType
    };
    Q_ENUM(AirConditioningError)

    explicit AirConditioningManager(ThingManager *thingManager, QObject *parent = nullptr);

    ZoneInfos zones() const;
    ZoneInfo zone(const QUuid &thermostatId);
    QPair<AirConditioningManager::AirConditioningError, ZoneInfo> addZone(const QString &name, const QList<ThingId> &thermostats, const QList<ThingId> windowSensors, const QList<ThingId> indoorSensors, const QList<ThingId> outdoorSensors, const QList<ThingId> notifications);
    AirConditioningError removeZone(const QUuid &zoneId);

    AirConditioningError setZoneName(const QUuid &zoneId, const QString &name);
    AirConditioningError setZoneStandbySetpoint(const QUuid &zoneId, double standbySetpoint);
    AirConditioningError setZoneSetpointOverride(const QUuid &zoneId, double setpoint, ZoneInfo::SetpointOverrideMode mode, uint minutes);
    AirConditioningError setZoneWeekSchedules(const QUuid &zoneId, const TemperatureWeekSchedule &temperatureWeekSchedule);

    AirConditioningError setZoneThings(const QUuid &zoneId, const QList<ThingId> &thermostats, const QList<ThingId> &windowSensors, const QList<ThingId> &indoorSensors, const QList<ThingId> &outdoorSensors, const QList<ThingId> &notifications);
//    AirConditioningError addThing(const QUuid &zoneId, const ThingId &thingId);
//    AirConditioningError removeThing(const QUuid &zoneId, const ThingId &thingId);


signals:
    void zoneAdded(const ZoneInfo &zone);
    void zoneRemoved(const QUuid &zoneId);
    void zoneChanged(const ZoneInfo &zoneInfo);
    void notificationThingsChanged(const QList<ThingId> &notificationThigns);

private slots:
    void onThingAdded(Thing *thing);
    void onThingRemoved(const ThingId &thingId);
    void onThingStateChaged(Thing *thing, const StateTypeId &stateTypeId, const QVariant &value, const QVariant &minValue, const QVariant &maxValue);
    void onActionExecuted(const Action &action, Thing::ThingError status);

    void update();
    void updateZone(const QUuid &zoneId);

private:
    void loadZones();
    void saveZones();

    AirConditioningError verifyThingIds(const QList<ThingId> &thermostats, const QList<ThingId> &windowSensors, const QList<ThingId> &indoorSensors, const QList<ThingId> &outdoorSensors, const QList<ThingId> &notifications);

private:
    ThingManager *m_thingManager = nullptr;
    QTimer *m_updateTimer = nullptr;

    QHash<ThingId, Thermostat*> m_thermostats;
    QHash<QUuid, ZoneInfo> m_zones;
    QHash<QUuid, ZoneInfo::ZoneStatus> m_eventualOverrideCache;
    QHash<ThingId, Notifications*> m_notifications;

    QDateTime m_lastUpdateTime;
};

#endif // AIRCONDITIONINGMANAGER_H
