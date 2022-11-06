/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2022, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
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
    ZoneInfo zone(const ThingId &thermostatId);
    QPair<AirConditioningManager::AirConditioningError, ZoneInfo> addZone(const QString &name, const QList<ThingId> &thermostats, const QList<ThingId> windowSensors, const QList<ThingId> indoorSensors, const QList<ThingId> outdoorSensors);
    AirConditioningError removeZone(const QUuid &zoneId);

    AirConditioningError setZoneName(const QUuid &zoneId, const QString &name);
    AirConditioningError setZoneStandbySetpoint(const QUuid &zoneId, double standbySetpoint);
    AirConditioningError setZoneSetpointOverride(const QUuid &zoneId, double setpoint, ZoneInfo::SetpointOverrideMode mode, uint minutes);
    AirConditioningError setZoneWeekSchedules(const QUuid &zoneId, const TemperatureWeekSchedule &temperatureWeekSchedule);

    AirConditioningError setZoneThings(const QUuid &zoneId, const QList<ThingId> &thermostats, const QList<ThingId> &windowSensors, const QList<ThingId> &indoorSensors, const QList<ThingId> &outdoorSensors);
//    AirConditioningError addThing(const QUuid &zoneId, const ThingId &thingId);
//    AirConditioningError removeThing(const QUuid &zoneId, const ThingId &thingId);


signals:
    void zoneAdded(const ZoneInfo &zone);
    void zoneRemoved(const QUuid &zoneId);
    void zoneChanged(const ZoneInfo &zoneInfo);

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

    AirConditioningError verifyThingIds(const QList<ThingId> &thermostats, const QList<ThingId> &windowSensors, const QList<ThingId> &indoorSensors, const QList<ThingId> &outdoorSensors);

private:
    ThingManager *m_thingManager = nullptr;
    QTimer *m_updateTimer = nullptr;

    QHash<ThingId, Thermostat*> m_thermostats;
    QHash<QUuid, ZoneInfo> m_zones;
    QHash<QUuid, ZoneInfo::ZoneStatus> m_eventualOverrideCache;

    QDateTime m_lastUpdateTime;
};

#endif // AIRCONDITIONINGMANAGER_H
