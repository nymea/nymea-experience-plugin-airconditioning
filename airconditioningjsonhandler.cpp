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

#include "airconditioningjsonhandler.h"
#include "airconditioningmanager.h"


Q_DECLARE_LOGGING_CATEGORY(dcAdaptiveLighting)

AirConditioningJsonHandler::AirConditioningJsonHandler(AirConditioningManager *manager, QObject *parent):
    JsonHandler(parent),
    m_manager(manager)
{
    registerEnum<AirConditioningManager::AirConditioningError>();
    registerFlag<ZoneInfo::ZoneStatusFlag, ZoneInfo::ZoneStatus>();
    registerEnum<ZoneInfo::SetpointOverrideMode>();
    registerObject<ZoneInfo, ZoneInfos>();
    registerObject<TemperatureSchedule, TemperatureDaySchedule>();
    registerList<TemperatureWeekSchedule, TemperatureDaySchedule>();

    QVariantMap params, returns;
    QString description;

    params.clear(); returns.clear();
    description = "Get all Zones.";
    params.insert("o:zoneId", enumValueName(Uuid));
    returns.insert("airConditioningError", enumRef<AirConditioningManager::AirConditioningError>());
    returns.insert("zones", objectRef<ZoneInfos>());
    registerMethod("GetZones", description, params, returns, Types::PermissionScopeControlThings);

    params.clear(); returns.clear();
    description = "Create a zones.";
    params.insert("name", enumValueName(String));
    params.insert("o:thermostats", QVariantList() << enumValueName(Uuid));
    params.insert("o:windowSensors", QVariantList() << enumValueName(Uuid));
    params.insert("o:indoorSensors", QVariantList() << enumValueName(Uuid));
    params.insert("o:outdoorSensors", QVariantList() << enumValueName(Uuid));
    params.insert("o:notifications", QVariantList() << enumValueName(Uuid));
    returns.insert("airConditioningError", enumRef<AirConditioningManager::AirConditioningError>());
    returns.insert("o:zone", objectRef<ZoneInfo>());
    registerMethod("AddZone", description, params, returns);

    params.clear(); returns.clear();
    description = "Remove a zone.";
    params.insert("zoneId", enumValueName(Uuid));
    returns.insert("airConditioningError", enumRef<AirConditioningManager::AirConditioningError>());
    registerMethod("RemoveZone", description, params, returns);

    params.clear(); returns.clear();
    description = "Set zone name.";
    params.insert("zoneId", enumValueName(Uuid));
    params.insert("name", enumValueName(String));
    returns.insert("airConditioningError", enumRef<AirConditioningManager::AirConditioningError>());
    registerMethod("SetZoneName", description, params, returns);

    params.clear(); returns.clear();
    description = "Set zone standby setpoint temperature.";
    params.insert("zoneId", enumValueName(Uuid));
    params.insert("standbySetpoint", enumValueName(Double));
    returns.insert("airConditioningError", enumRef<AirConditioningManager::AirConditioningError>());
    registerMethod("SetZoneStandbySetpoint", description, params, returns);

    params.clear(); returns.clear();
    description = "Set zone setpoint override temperature. Parameter minutes gives the minutes until the setpoint should return to the standby/schedule.";
    params.insert("zoneId", enumValueName(Uuid));
    params.insert("setpointOverride", enumValueName(Double));
    params.insert("mode", enumRef<ZoneInfo::SetpointOverrideMode>());
    params.insert("o:minutes", enumValueName(Int));
    returns.insert("airConditioningError", enumRef<AirConditioningManager::AirConditioningError>());
    registerMethod("SetZoneSetpointOverride", description, params, returns, Types::PermissionScopeControlThings);

    params.clear(); returns.clear();
    description = "Set the time schedule for a zone.";
    params.insert("zoneId", enumValueName(Uuid));
    params.insert("weekSchedule", objectRef<TemperatureWeekSchedule>());
    returns.insert("airConditioningError", enumRef<AirConditioningManager::AirConditioningError>());
    registerMethod("SetZoneWeekSchedule", description, params, returns, Types::PermissionScopeControlThings);

    params.clear(); returns.clear();
    description = "Set Zone things";
    params.insert("zoneId", enumValueName(Uuid));
    params.insert("o:thermostats", QVariantList() << enumValueName(Uuid));
    params.insert("o:windowSensors", QVariantList() << enumValueName(Uuid));
    params.insert("o:indoorSensors", QVariantList() << enumValueName(Uuid));
    params.insert("o:outdoorSensors", QVariantList() << enumValueName(Uuid));
    params.insert("o:notifications", QVariantList() << enumValueName(Uuid));
    returns.insert("airConditioningError", enumRef<AirConditioningManager::AirConditioningError>());
    registerMethod("SetZoneThings", description, params, returns);

    params.clear();
    description = "Emitted whenever a zone is added";
    params.insert("zone", objectRef<ZoneInfo>());
    registerNotification("ZoneAdded", description, params);

    params.clear();
    description = "Emitted whenever a zone is removed";
    params.insert("zoneId", enumValueName(Uuid));
    registerNotification("ZoneRemoved", description, params);

    params.clear();
    description = "Emitted whenever a zone changes";
    params.insert("zone", objectRef<ZoneInfo>());
    registerNotification("ZoneChanged", description, params);

    connect(manager, &AirConditioningManager::zoneAdded, this, [=](const ZoneInfo &zone){
        emit ZoneAdded({{"zone", pack(zone)}});
    });
    connect(manager, &AirConditioningManager::zoneRemoved, this, [=](const QUuid &zoneId){
        emit ZoneRemoved({{"zoneId", zoneId}});
    });
    connect(manager, &AirConditioningManager::zoneChanged, this, [=](const ZoneInfo &zone){
        emit ZoneChanged({{"zone", pack(zone)}});
    });
}

QString AirConditioningJsonHandler::name() const
{
    return "AirConditioning";
}

JsonReply *AirConditioningJsonHandler::GetZones(const QVariantMap &params)
{
    ZoneInfos zones;
    if (params.contains("zoneId")) {
        QUuid zoneId = ThingId(params.value("zoneId").toUuid());
        zones = ZoneInfos({m_manager->zone(zoneId)});
    } else {
        zones = m_manager->zones();
    }
    return createReply({
                           {"airConditioningError", enumValueName(AirConditioningManager::AirConditioningErrorNoError)},
                           {"zones", pack(zones)}
                       });
}

JsonReply *AirConditioningJsonHandler::AddZone(const QVariantMap &params)
{
    QList<ThingId> thermostats, windowSensors, indoorSensors, outdoorSensors, notifications;
    foreach (const QVariant &id, params.value("thermostats").toList()) {
        thermostats.append(id.toUuid());
    }
    foreach (const QVariant &id, params.value("windowSensors").toList()) {
        windowSensors.append(id.toUuid());
    }
    foreach (const QVariant &id, params.value("indoorSensors").toList()) {
        indoorSensors.append(id.toUuid());
    }
    foreach (const QVariant &id, params.value("outdoorSensors").toList()) {
        outdoorSensors.append(id.toUuid());
    }
    foreach (const QVariant &id, params.value("notificatiosn").toList()) {
        notifications.append(id.toUuid());
    }
    QPair<AirConditioningManager::AirConditioningError, ZoneInfo> status = m_manager->addZone(params.value("name").toString(), thermostats, windowSensors, indoorSensors, outdoorSensors, notifications);
    QVariantMap ret = {
        {"airConditioningError", enumValueName(status.first)}
    };
    if (status.first == AirConditioningManager::AirConditioningErrorNoError) {
        ret.insert("zone", pack(status.second));
    }
    return createReply(ret);
}

JsonReply *AirConditioningJsonHandler::RemoveZone(const QVariantMap &params)
{
    AirConditioningManager::AirConditioningError status = m_manager->removeZone(params.value("zoneId").toUuid());
    return createReply({{"airConditioningError", enumValueName(status)}});
}

JsonReply *AirConditioningJsonHandler::SetZoneName(const QVariantMap &params)
{
    AirConditioningManager::AirConditioningError status = m_manager->setZoneName(params.value("zoneId").toUuid(), params.value("name").toString());
    return createReply({
                           {"airConditioningError", enumValueName(status)}
                       });
}

JsonReply *AirConditioningJsonHandler::SetZoneStandbySetpoint(const QVariantMap &params)
{
    QUuid zoneId = params.value("zoneId").toUuid();
    double standbySetpoint = params.value("standbySetpoint").toDouble();

    AirConditioningManager::AirConditioningError status = m_manager->setZoneStandbySetpoint(zoneId, standbySetpoint);
    return createReply({{"airConditioningError", enumValueName(status)}});
}

JsonReply *AirConditioningJsonHandler::SetZoneSetpointOverride(const QVariantMap &params)
{
    QUuid zoneId = params.value("zoneId").toUuid();
    double standbySetpoint = params.value("setpointOverride").toDouble();
    QMetaEnum modeEnum = QMetaEnum::fromType<ZoneInfo::SetpointOverrideMode>();
    ZoneInfo::SetpointOverrideMode mode = static_cast<ZoneInfo::SetpointOverrideMode>(modeEnum.keyToValue(params.value("mode").toByteArray()));
    uint minutes = params.value("minutes", 0).toUInt();

    AirConditioningManager::AirConditioningError status = m_manager->setZoneSetpointOverride(zoneId, standbySetpoint, mode, minutes);
    return createReply({{"airConditioningError", enumValueName(status)}});
}

JsonReply *AirConditioningJsonHandler::SetZoneWeekSchedule(const QVariantMap &params)
{
    TemperatureWeekSchedule weekSchedule = unpack<TemperatureWeekSchedule>(params.value("weekSchedule"));
    QUuid zoneId = params.value("zoneId").toUuid();
    AirConditioningManager::AirConditioningError status = m_manager->setZoneWeekSchedules(zoneId, weekSchedule);
    return createReply({{"airConditioningError", enumValueName(status)}});
}

JsonReply *AirConditioningJsonHandler::SetZoneThings(const QVariantMap &params)
{
    QUuid zoneId = params.value("zoneId").toUuid();

    ZoneInfo zone = m_manager->zone(zoneId);

    QList<ThingId> thermostats, windowSensors, indoorSensors, outdoorSensors, notifications;
    if (params.contains("thermostats")) {
        foreach (const QVariant &variant, params.value("thermostats").toList()) {
            thermostats.append(ThingId(variant.toUuid()));
        }
    } else {
        thermostats = zone.thermostats();
    }

    if (params.contains("windowSensors")) {
        foreach (const QVariant &variant, params.value("windowSensors").toList()) {
            windowSensors.append(ThingId(variant.toUuid()));
        }
    } else {
        windowSensors = zone.windowSensors();
    }

    if (params.contains("indoorSensors")) {
        foreach (const QVariant &variant, params.value("indoorSensors").toList()) {
            indoorSensors.append(ThingId(variant.toUuid()));
        }
    } else {
        indoorSensors = zone.indoorSensors();
    }

    if (params.contains("outdoorSensors")) {
        foreach (const QVariant &variant, params.value("outdoorSensors").toList()) {
            outdoorSensors.append(ThingId(variant.toUuid()));
        }
    } else {
        outdoorSensors = zone.outdoorSensors();
    }

    if (params.contains("notifications")) {
        foreach (const QVariant &variant, params.value("notifications").toList()) {
            notifications.append(ThingId(variant.toUuid()));
        }
    } else {
        notifications = zone.notifications();
    }

    AirConditioningManager::AirConditioningError status = m_manager->setZoneThings(zoneId, thermostats, windowSensors, indoorSensors, outdoorSensors, notifications);
    return createReply({{"airConditioningError", enumValueName(status)}});
}
