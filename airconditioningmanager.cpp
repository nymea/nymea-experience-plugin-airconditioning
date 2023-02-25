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

#include "airconditioningmanager.h"
#include "zoneinfo.h"

#include <nymeasettings.h>

#include <QMetaEnum>
#include <qmath.h>

Q_DECLARE_LOGGING_CATEGORY(dcAirConditioning)

AirConditioningManager::AirConditioningManager(ThingManager *thingManager, QObject *parent):
    QObject(parent),
    m_thingManager(thingManager)
{
    qCDebug(dcAirConditioning()) << "Loading air conditioning experience...";
    connect(m_thingManager, &ThingManager::thingAdded, this, &AirConditioningManager::onThingAdded);
    connect(m_thingManager, &ThingManager::thingRemoved, this, &AirConditioningManager::onThingRemoved);
    connect(m_thingManager, &ThingManager::thingStateChanged, this, &AirConditioningManager::onThingStateChaged);
    connect(m_thingManager, &ThingManager::actionExecuted, this, &AirConditioningManager::onActionExecuted);

    foreach (Thing *thing, m_thingManager->configuredThings()) {
        if (thing->thingClass().interfaces().contains("thermostat")) {
            m_thermostats.insert(thing->id(), new Thermostat(m_thingManager, thing, this));
        }
        if (thing->thingClass().interfaces().contains("notifications")) {
            m_notifications.insert(thing->id(), new Notifications(m_thingManager, thing, this));
        }
    }

    loadZones();

    m_updateTimer = new QTimer(this);
    m_updateTimer->start(1000);
    connect(m_updateTimer, &QTimer::timeout, this, [=](){
        if (m_lastUpdateTime.time().minute() != QDateTime::currentDateTime().time().minute()) {
            m_lastUpdateTime = QDateTime::currentDateTime();
            update();
        }
    });
}

ZoneInfos AirConditioningManager::zones() const
{
    return m_zones.values();
}

ZoneInfo AirConditioningManager::zone(const QUuid &zoneId)
{
    return m_zones.value(zoneId);
}

QPair<AirConditioningManager::AirConditioningError, ZoneInfo> AirConditioningManager::addZone(const QString &name, const QList<ThingId> &thermostats, const QList<ThingId> windowSensors, const QList<ThingId> indoorSensors, const QList<ThingId> outdoorSensors, const QList<ThingId> notifications)
{
    ZoneInfo zone(QUuid::createUuid());
    zone.setName(name);
    zone.setWeekSchedule(TemperatureWeekSchedule::create());
    AirConditioningError status = verifyThingIds(thermostats, windowSensors, indoorSensors, outdoorSensors, notifications);
    if (status != AirConditioningErrorNoError) {
        qCWarning(dcAirConditioning()) << "Invalid thing id" << status << "in" << thermostats;
        return qMakePair<AirConditioningError, ZoneInfo>(status, ZoneInfo());
    }

    zone.setThermostats(thermostats);
    zone.setWindowSensors(windowSensors);
    zone.setIndoorSensors(indoorSensors);
    zone.setOutdoorSensors(outdoorSensors);
    zone.setNotifications(notifications);

    m_zones.insert(zone.id(), zone);
    saveZones();

    emit zoneAdded(zone);
    return qMakePair<AirConditioningError, ZoneInfo>(AirConditioningErrorNoError, zone);
}

AirConditioningManager::AirConditioningError AirConditioningManager::removeZone(const QUuid &zoneId)
{
    if (!m_zones.contains(zoneId)) {
        return AirConditioningErrorZoneNotFound;
    }
    m_zones.remove(zoneId);
    saveZones();

    emit zoneRemoved(zoneId);
    return AirConditioningErrorNoError;
}

AirConditioningManager::AirConditioningError AirConditioningManager::setZoneName(const QUuid &zoneId, const QString &name)
{
    if (!m_zones.contains(zoneId)) {
        return AirConditioningErrorZoneNotFound;
    }
    m_zones[zoneId].setName(name);
    saveZones();

    emit zoneChanged(m_zones.value(zoneId));
    return AirConditioningErrorNoError;
}

AirConditioningManager::AirConditioningError AirConditioningManager::setZoneStandbySetpoint(const QUuid &zoneId, double standbySetpoint)
{
    if (!m_zones.contains(zoneId)) {
        return AirConditioningErrorZoneNotFound;
    }
    m_zones[zoneId].setStandbySetpoint(standbySetpoint);

    saveZones();

    emit zoneChanged(m_zones.value(zoneId));

    update();

    return AirConditioningErrorNoError;
}

AirConditioningManager::AirConditioningError AirConditioningManager::setZoneWeekSchedules(const QUuid &zoneId, const TemperatureWeekSchedule &weekSchedule)
{
    if (!m_zones.contains(zoneId)) {
        return AirConditioningErrorZoneNotFound;
    }

    if (weekSchedule.count() != 7) {
        qCWarning(dcAirConditioning()) << "There must be exactly 7 schedules in a week schedule:" << weekSchedule;
        return AirConditioningErrorInvalidTimeSpec;
    }
    for (int day = 0; day < 7; day++) {
        TemperatureDaySchedule daySchedule = weekSchedule.at(day);
        for (int i = 0; i < daySchedule.count(); i++) {
            TemperatureSchedule schedule = daySchedule.at(i);
            if (schedule.startTime() >= schedule.endTime()) {
                qCWarning(dcAirConditioning()) << "Invalid time spec. startTime mus be earlier than endTime:" << schedule;
                return AirConditioningErrorInvalidTimeSpec;
            }
            for (int j = i+1; j < daySchedule.count(); j++) {
                TemperatureSchedule other = daySchedule.at(j);
                if (other.startTime() < schedule.startTime() && other.endTime() > schedule.startTime()) {
                    qCWarning(dcAirConditioning()) << "Invalid time spec. Overlapping schedules:\n" << schedule << "\n" << other;
                    return AirConditioningErrorInvalidTimeSpec;
                }
                if (other.startTime() < schedule.endTime() && other.endTime() > schedule.startTime()) {
                    qCWarning(dcAirConditioning()) << "Invalid time spec. Overlapping schedules:\n" << schedule << "\n" << other;
                    return AirConditioningErrorInvalidTimeSpec;
                }
            }
        }
    }

    m_zones[zoneId].setWeekSchedule(weekSchedule);
    saveZones();
    emit zoneChanged(m_zones.value(zoneId));
    qCInfo(dcAirConditioning()) << "Temperature schedule saved:" << weekSchedule;
    update();
    return AirConditioningErrorNoError;
}

AirConditioningManager::AirConditioningError AirConditioningManager::setZoneThings(const QUuid &zoneId, const QList<ThingId> &thermostats, const QList<ThingId> &windowSensors, const QList<ThingId> &indoorSensors, const QList<ThingId> &outdoorSensors, const QList<ThingId> &notifications)
{
    if (!m_zones.contains(zoneId)) {
        return AirConditioningErrorZoneNotFound;
    }
    AirConditioningError status = verifyThingIds(thermostats, windowSensors, indoorSensors, outdoorSensors, notifications);
    if (status != AirConditioningErrorNoError) {
        return status;
    }
    m_zones[zoneId].setThermostats(thermostats);
    m_zones[zoneId].setWindowSensors(windowSensors);
    m_zones[zoneId].setIndoorSensors(indoorSensors);
    m_zones[zoneId].setOutdoorSensors(outdoorSensors);
    m_zones[zoneId].setNotifications(notifications);
    saveZones();
    qCDebug(dcAirConditioning()) << "Zone things set. Thermostats:" << thermostats << "Window sensors:" << windowSensors << "indoor sensors:" << indoorSensors << "outdoor sensors:" << outdoorSensors << "notifications:" << notifications;
    emit zoneChanged(m_zones.value(zoneId));
    updateZone(zoneId);
    return AirConditioningErrorNoError;
}

AirConditioningManager::AirConditioningError AirConditioningManager::setZoneSetpointOverride(const QUuid &zoneId, double setpoint, ZoneInfo::SetpointOverrideMode mode, uint minutes)
{
    if (!m_zones.contains(zoneId)) {
        return AirConditioningErrorZoneNotFound;
    }
    m_zones[zoneId].setSetpointOverride(setpoint, mode, QDateTime::currentDateTime().addMSecs(minutes * 60000));
    m_eventualOverrideCache[zoneId] = (m_zones[zoneId].zoneStatus() | ZoneInfo::ZoneStatusFlagSetpointOverrideActive);
    qCDebug(dcAirConditioning()) << "Memorizing zone status:" << m_eventualOverrideCache.value(zoneId);
    saveZones();
    emit zoneChanged(m_zones.value(zoneId));
    updateZone(zoneId);
    return AirConditioningErrorNoError;
}

void AirConditioningManager::onThingAdded(Thing *thing)
{
    if (thing->thingClass().interfaces().contains("thermostat")) {
        qCInfo(dcAirConditioning()) << "Thermostat added:" << thing;
        m_thermostats.insert(thing->id(), new Thermostat(m_thingManager, thing, this));
    }
    if (thing->thingClass().interfaces().contains("notifications")) {
        qCInfo(dcAirConditioning()) << "Notifications added:" << thing;
        m_notifications.insert(thing->id(), new Notifications(m_thingManager, thing, this));
    }
}

void AirConditioningManager::onThingRemoved(const ThingId &thingId)
{
    foreach (const ZoneInfo &zone, m_zones) {
        QList<ThingId> thermostats = m_zones.value(zone.id()).thermostats();
        QList<ThingId> windowSensors = m_zones.value(zone.id()).windowSensors();
        QList<ThingId> indoorSensors = m_zones.value(zone.id()).indoorSensors();
        QList<ThingId> outdoorSensors = m_zones.value(zone.id()).outdoorSensors();
        QList<ThingId> notifications = m_zones.value(zone.id()).notifications();
        bool changed = false;
        if (thermostats.contains(thingId)) {
            thermostats.removeAll(thingId);
            changed = true;
        }
        if (windowSensors.contains(thingId)) {
            windowSensors.removeAll(thingId);
            changed = true;
        }
        if (indoorSensors.contains(thingId)) {
            indoorSensors.removeAll(thingId);
            changed = true;
        }
        if (outdoorSensors.contains(thingId)) {
            outdoorSensors.removeAll(thingId);
            changed = true;
        }
        if (notifications.contains(thingId)) {
            notifications.removeAll(thingId);
            changed = true;
        }
        if (changed) {
            setZoneThings(zone.id(), thermostats, windowSensors, indoorSensors, outdoorSensors, notifications);
        }
    }
}

void AirConditioningManager::onThingStateChaged(Thing *thing, const StateTypeId &stateTypeId, const QVariant &value, const QVariant &minValue, const QVariant &maxValue)
{
    Q_UNUSED(minValue)
    Q_UNUSED(maxValue)

    StateType stateType = thing->thingClass().getStateType(stateTypeId);
    foreach (const ZoneInfo &zone, m_zones) {
        bool changed = false;
        if (zone.windowSensors().contains(thing->id()) && stateType.name() == "closed") {
            qCDebug(dcAirConditioning()) << "Window sensor in zone" << zone.name() << "changed" << value;
            changed = true;
        }
        if (zone.thermostats().contains(thing->id()) && stateType.name() == "temperature") {
            qCDebug(dcAirConditioning()) << "Thermostat temperature sensor in zone" << zone.name() << "changed" << value;
            changed = true;
        }
        if (zone.indoorSensors().contains(thing->id()) && QStringList{"temperature", "humidity", "voc", "pm25"}.contains(stateType.name())) {
            qCDebug(dcAirConditioning()) << "Sensor for" << stateType.name() << "in zone" << zone.name() << "changed" << value;
            changed = true;
        }
        if (changed) {
            updateZone(zone.id());
        }
    }
}

void AirConditioningManager::onActionExecuted(const Action &action, Thing::ThingError status)
{
    if (action.triggeredBy() == Action::TriggeredByUser && status == Thing::ThingErrorNoError) {
        Thing *thing = m_thingManager->findConfiguredThing(action.thingId());
        if (thing && thing->thingClass().interfaces().contains("thermostat")) {
            if (thing->thingClass().actionTypes().findById(action.actionTypeId()).name() == "targetTemperature") {
                foreach (const ZoneInfo &zone, m_zones) {
                    if (zone.thermostats().contains(thing->id())) {
                        qCInfo(dcAirConditioning()).nospace() << "Target temperature changed on thermostat in zone " << zone.name() << ". Activating setpoint override for" << action.paramValue(action.actionTypeId()).toDouble();
                        m_zones[zone.id()].setSetpointOverride(action.paramValue(action.actionTypeId()).toDouble(), ZoneInfo::SetpointOverrideModeEventual);
                    }
                }
            }
        }
    }
}

void AirConditioningManager::update()
{
    qCDebug(dcAirConditioning()) << "Upadting air conditioning";
    foreach (const QUuid &zoneId, m_zones.keys()) {
        updateZone(zoneId);
    }
}

void AirConditioningManager::updateZone(const QUuid &zoneId)
{
    ZoneInfo zone = m_zones.value(zoneId);
    qCDebug(dcAirConditioning()) << "*** Evaluating Zone:" << zone.name();

    QDateTime now = QDateTime::currentDateTime();

    bool timeScheduleActive = false;
    bool overrideActive = false;
    qCDebug(dcAirConditioning()) << "Standby temp:" << zone.standbySetpoint() << "Override:" << zone.setpointOverrideMode() << zone.setpointOverride() << zone.setpointOverrideEnd().toString(Qt::DefaultLocaleShortDate) << "Schedules:" << zone.weekSchedule();

    if (zone.setpointOverrideMode() == ZoneInfo::SetpointOverrideModeUnlimited
            || (zone.setpointOverrideMode() == ZoneInfo::SetpointOverrideModeTimed && zone.setpointOverrideEnd() > now)
            || zone.setpointOverrideMode() == ZoneInfo::SetpointOverrideModeEventual) {
        qCDebug(dcAirConditioning()) << "Setpoint override active until" << zone.setpointOverrideEnd();
        overrideActive = true;
    }

    TemperatureDaySchedule daySchedule = zone.weekSchedule().at(now.date().dayOfWeek() - 1);
    double timeScheduleTemp;
    foreach (const TemperatureSchedule &schedule, daySchedule) {
        if (schedule.startTime() < now.time() && schedule.endTime() > now.time()) {
            qCDebug(dcAirConditioning()) << "Schedule is active:" << schedule;
            timeScheduleTemp = schedule.temperature();
            timeScheduleActive = true;
            break;
        }
    }

    // Checking window open
    bool windowOpen = false;
    foreach (const ThingId &thingId, zone.windowSensors()) {
        Thing *thing = m_thingManager->findConfiguredThing(thingId);
        if (!thing) {
            qCWarning(dcAirConditioning()) << "Thing" << thingId << "seems to have been removed from the system!";
            continue;
        }
        if (!thing->stateValue("closed").toBool()) {
            qCInfo(dcAirConditioning()) << "Window open.";
            windowOpen = true;
            break;
        }
    }


    // ***********

    double targetTemp = zone.standbySetpoint();
    if (overrideActive) {
        targetTemp = zone.setpointOverride();
    } else if (timeScheduleActive) {
        targetTemp = timeScheduleTemp;
    }

    qCDebug(dcAirConditioning()) << "Window open" << windowOpen << "Override active:" << overrideActive << "Time schedule active:" << timeScheduleActive << "target:" << targetTemp;

    // To determine the zone temperature we'll first check the thermostats if they have a temp sensor and use the highest value
    // If no thermstats with temp sensors are available, we'll use the highest temp value from the indoor sensors.
    bool tempFromThermostat = false;
    bool tempFromSensors = false;
    double temperature = 0;

    foreach (const ThingId &thingId, zone.thermostats()) {
        Thermostat *thermostat = m_thermostats.value(thingId);
        if (thermostat) {
            qCDebug(dcAirConditioning()) << "Setting window open" << windowOpen << " and target temp" << targetTemp;
            thermostat->setWindowOpen(windowOpen);
            thermostat->setTargetTemperature(targetTemp);

            if (thermostat->hasTemperatureSensor()) {
                qCDebug(dcAirConditioning()) << "Thermostat has temperature sensor:" << thermostat->temperature();
                if (!tempFromThermostat) {
                    temperature = thermostat->temperature();
                    tempFromThermostat = true;
                }
                temperature = qMax(temperature, thermostat->temperature());
            }
        }
    }

    double humidity = 0;
    uint voc = 0;
    double pm25 = 0;

    foreach (const ThingId &thingId, zone.indoorSensors()) {
        Thing *thing = m_thingManager->findConfiguredThing(thingId);

        if (!tempFromThermostat) {
            if (thing->thingClass().interfaces().contains("temperaturesensor")) {
                if (!tempFromSensors) {
                    temperature = thing->stateValue("temperature").toDouble();
                    tempFromSensors = true;
                } else {
                    temperature = qMax(temperature, thing->stateValue("temperature").toDouble());
                }
            }
        }

        if (thing->thingClass().interfaces().contains("humiditysensor")) {
            humidity = qMax(humidity, thing->stateValue("humidity").toDouble());
        }

        if (thing->thingClass().interfaces().contains("vocsensor")) {
            voc = qMax(voc, thing->stateValue("voc").toUInt());
        }

        if (thing->thingClass().interfaces().contains("pm25sensor")) {
            pm25 = qMax(pm25, thing->stateValue("pm25").toDouble());
        }
    }

    ZoneInfo::ZoneStatus newStatus = ZoneInfo::ZoneStatusFlagNone;
    newStatus.setFlag(ZoneInfo::ZoneStatusFlagWindowOpen, windowOpen);
    newStatus.setFlag(ZoneInfo::ZoneStatusFlagSetpointOverrideActive, overrideActive);
    newStatus.setFlag(ZoneInfo::ZoneStatusFlagTimeScheduleActive, timeScheduleActive);
    newStatus.setFlag(ZoneInfo::ZoneStatusFlagHighHumidity, humidity >= 65); // > 60 over longer periods of time may cause mould, 70 will cause mould
    newStatus.setFlag(ZoneInfo::ZoneStatusFlagBadAir, voc >= 660 || pm25 >= 25); // VOC: 660 Moderate as of IAQ, PM25: 25 Moderate as of CAQI

    if (zone.setpointOverrideMode() == ZoneInfo::SetpointOverrideModeEventual &&
            newStatus != m_eventualOverrideCache.value(zone.id())) {
        qCDebug(dcAirConditioning()) << "Zone status changed:" << m_eventualOverrideCache.value(zone.id()) << "->" << newStatus << "Resetting eventual override";
        m_zones[zone.id()].setSetpointOverride(zone.setpointOverride(), ZoneInfo::SetpointOverrideModeNone);
        updateZone(zone.id());
        return;

    }

    if (targetTemp != zone.currentSetpoint()
            || newStatus != zone.zoneStatus()
            || temperature != zone.temperature()
            || humidity != zone.humidity()
            || voc != zone.voc()
            || pm25 != zone.pm25()
            ) {
        qCDebug(dcAirConditioning()) << "Modifying Zone: setpoint:" << targetTemp << "status:" << newStatus << "temp:" << temperature << "humidity:" << humidity << "VOC:" << voc << "PM25:" << pm25;
        m_zones[zone.id()].setCurrentSetpoint(targetTemp);
        m_zones[zone.id()].setZoneStatus(newStatus);
        m_zones[zone.id()].setTemperature(temperature);
        m_zones[zone.id()].setHumidity(humidity);
        m_zones[zone.id()].setVoc(voc);
        m_zones[zone.id()].setPm25(pm25);
        emit zoneChanged(m_zones.value(zone.id()));

        foreach (const ThingId &notificationThingId, zone.notifications()) {
            Notifications *notifications = m_notifications.value(notificationThingId);
            if (!notifications) {
                qCWarning(dcAirConditioning()) << "Stale notification thing id in zone!" << notificationThingId << m_notifications.keys();
                continue;
            }
            notifications->update(m_zones[zone.id()]);
        }
    }
}

void AirConditioningManager::loadZones()
{
    qCDebug(dcAirConditioning()) << "Loading zones";
    QSettings settings(NymeaSettings::settingsPath() + "/airconditioning.conf", QSettings::IniFormat);

    settings.beginGroup("zones");
    qCDebug(dcAirConditioning()) << "child groups of zones" << settings.childKeys() << settings.childGroups();
    foreach (const QString &key, settings.childGroups()) {
        settings.beginGroup(key);
        qCDebug(dcAirConditioning()) << "Loading zone" << key;
        QUuid zoneId(key);
        ZoneInfo zone(zoneId);
        zone.setName(settings.value("name").toString());
        QMetaEnum modeEnum = QMetaEnum::fromType<ZoneInfo::SetpointOverrideMode>();
        ZoneInfo::SetpointOverrideMode mode = static_cast<ZoneInfo::SetpointOverrideMode>(modeEnum.keyToValue(settings.value("setpointOverrideMode", "SetpointOverrideModeNone").toByteArray()));
        zone.setSetpointOverride(settings.value("setpointOverride").toDouble(), mode, settings.value("setpointOverrideEnd").toDateTime());
        zone.setStandbySetpoint(settings.value("standbySetpoint").toDouble());
        settings.beginGroup("weekSchedule");
        TemperatureWeekSchedule weekSchedule;
        for (int day = 0; day < 7; day++) {
            TemperatureDaySchedule daySchedule;
            settings.beginGroup(QString::number(day));
            foreach (const QString &childGroup, settings.childGroups()) {
                settings.beginGroup(childGroup);
                QTime startTime = settings.value("startTime").toTime();
                QTime endTime = settings.value("endTime").toTime();
                double temperature = settings.value("temperature").toDouble();
                TemperatureSchedule schedule(startTime, endTime, temperature);
                daySchedule.append(schedule);
                settings.endGroup(); // schedule
            }
            weekSchedule.append(daySchedule);
            settings.endGroup(); // daySchedule
        }
        settings.endGroup(); // weekSchedule
        zone.setWeekSchedule(weekSchedule);

        QList<ThingId> thermostats, windowSensors, indoorSensors, outdoorSensors, notifications;
        foreach (const QString &thingId, settings.value("thermostats").toStringList()) {
            thermostats.append(ThingId(thingId));
        }
        zone.setThermostats(thermostats);
        foreach (const QString &thingId, settings.value("windowSensors").toStringList()) {
            windowSensors.append(ThingId(thingId));
        }
        zone.setWindowSensors(windowSensors);
        foreach (const QString &thingId, settings.value("indoorSensors").toStringList()) {
            indoorSensors.append(ThingId(thingId));
        }
        zone.setIndoorSensors(indoorSensors);
        foreach (const QString &thingId, settings.value("outdoorSensors").toStringList()) {
            outdoorSensors.append(ThingId(thingId));
        }
        zone.setOutdoorSensors(outdoorSensors);
        foreach (const QString &thingId, settings.value("notifications").toStringList()) {
            notifications.append(ThingId(thingId));
        }
        zone.setNotifications(notifications);

        qCDebug(dcAirConditioning()) << "Zone Loaded:" << zone.thermostats() << zone.notifications();
        m_zones.insert(zoneId, zone);
        settings.endGroup(); // zone
    }
    settings.endGroup(); // zones
}

void AirConditioningManager::saveZones()
{
    qCDebug(dcAirConditioning()) << "Saving zones";
    QSettings settings(NymeaSettings::settingsPath() + "/airconditioning.conf", QSettings::IniFormat);
    settings.beginGroup("zones");
    settings.clear();
    foreach (const ZoneInfo &zone, m_zones) {
        settings.beginGroup(zone.id().toString());
        settings.setValue("name", zone.name());
        settings.setValue("standbySetpoint", zone.standbySetpoint());
        settings.setValue("setpointOverride", zone.setpointOverride());
        QMetaEnum modeEnum = QMetaEnum::fromType<ZoneInfo::SetpointOverrideMode>();
        settings.setValue("setpointOverrideMode", modeEnum.valueToKey(zone.setpointOverrideMode()));
        settings.setValue("setpointOverrideEnd", zone.setpointOverrideEnd());

        settings.beginGroup("weekSchedule");
        for (int day = 0; day < 7; day++) {
            settings.beginGroup(QString::number(day));
            TemperatureDaySchedule daySchedule = zone.weekSchedule().at(day);
            for (int i = 0; i < daySchedule.count(); i++) {
                TemperatureSchedule schedule = daySchedule.at(i);
                settings.beginGroup(QString::number(i));
                settings.setValue("startTime", schedule.startTime());
                settings.setValue("endTime", schedule.endTime());
                settings.setValue("temperature", schedule.temperature());
                settings.endGroup(); // schedule
            }
            settings.endGroup(); // daySchedule
        }
        settings.endGroup(); // weekSchedule

        QStringList thermostats, windowSensors, indoorSensors, outdoorSensors, notifications;
        foreach (const ThingId &thingId, zone.thermostats()) {
            thermostats.append(thingId.toString());
        }
        settings.setValue("thermostats", thermostats);
        foreach (const ThingId &thingId, zone.windowSensors()) {
            windowSensors.append(thingId.toString());
        }
        settings.setValue("windowSensors", windowSensors);
        foreach (const ThingId &thingId, zone.indoorSensors()) {
            indoorSensors.append(thingId.toString());
        }
        settings.setValue("indoorSensors", indoorSensors);
        foreach (const ThingId &thingId, zone.outdoorSensors()) {
            outdoorSensors.append(thingId.toString());
        }
        settings.setValue("outdoorSensors", outdoorSensors);

        foreach (const ThingId &thingId, zone.notifications()) {
            notifications.append(thingId.toString());
        }
        settings.setValue("notifications", notifications);

        settings.endGroup(); // zone
    }
    settings.endGroup();

}

AirConditioningManager::AirConditioningError AirConditioningManager::verifyThingIds(const QList<ThingId> &thermostats, const QList<ThingId> &windowSensors, const QList<ThingId> &indoorSensors, const QList<ThingId> &outdoorSensors, const QList<ThingId> &notifications)
{
    foreach (const QUuid &thingId, thermostats) {
        Thing *thing = m_thingManager->findConfiguredThing(thingId);
        if (!thing) {
            qCWarning(dcAirConditioning()) << "No thing with id" << thingId;
            return AirConditioningErrorThingNotFound;
        }
        if (!thing->thingClass().interfaces().contains("thermostat")) {
            qCWarning(dcAirConditioning()) << "Not a thermostat:" << thing->name();
            return AirConditioningErrorInvalidThingType;
        }
    }
    foreach (const QUuid &thingId, windowSensors) {
        Thing *thing = m_thingManager->findConfiguredThing(thingId);
        if (!thing) {
            qCWarning(dcAirConditioning()) << "No thing with id" << thingId;
            return AirConditioningErrorThingNotFound;
        }
        if (!thing->thingClass().interfaces().contains("closablesensor")) {
            qCWarning(dcAirConditioning()) << "Not a window sensor:" << thing->name();
            return AirConditioningErrorInvalidThingType;
        }
    }
    foreach (const QUuid &thingId, indoorSensors + outdoorSensors) {
        Thing *thing = m_thingManager->findConfiguredThing(thingId);
        if (!thing) {
            qCWarning(dcAirConditioning()) << "No thing with id" << thingId;
            return AirConditioningErrorThingNotFound;
        }
        if (!thing->thingClass().interfaces().contains("temperaturesensor")
                && !thing->thingClass().interfaces().contains("humiditysensor")
                && !thing->thingClass().interfaces().contains("vocsensor")
                && !thing->thingClass().interfaces().contains("pm25sensor")) {
            qCWarning(dcAirConditioning()) << "Not a temperature, humidity, voc or pm25 sensor:" << thing->name();
            return AirConditioningErrorInvalidThingType;
        }
    }
    foreach (const QUuid &thingId, notifications) {
        Thing *thing = m_thingManager->findConfiguredThing(thingId);
        if (!thing) {
            qCWarning(dcAirConditioning()) << "No thing with id" << thingId;
            return AirConditioningErrorThingNotFound;
        }
        if (!thing->thingClass().interfaces().contains("notifications")) {
            qCWarning(dcAirConditioning()) << "Not a notification thing:" << thing->name();
            return AirConditioningErrorInvalidThingType;
        }
    }
    return AirConditioningErrorNoError;
}

