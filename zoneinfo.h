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

#ifndef ZONEINFO_H
#define ZONEINFO_H

#include <QObject>
#include <QUuid>
#include <QVariant>

#include <typeutils.h>

#include "temperatureschedule.h"

class ZoneInfo
{
    Q_GADGET
    Q_PROPERTY(QUuid id READ id)
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(double currentSetpoint READ currentSetpoint)
    Q_PROPERTY(double standbySetpoint READ standbySetpoint)
    Q_PROPERTY(SetpointOverrideMode setpointOverrideMode READ setpointOverrideMode)
    Q_PROPERTY(double setpointOverride READ setpointOverride)
    Q_PROPERTY(QDateTime setpointOverrideEnd READ setpointOverrideEnd)
    Q_PROPERTY(QList<ThingId> thermostats READ thermostats)
    Q_PROPERTY(QList<ThingId> windowSensors READ windowSensors)
    Q_PROPERTY(QList<ThingId> indoorSensors READ indoorSensors)
    Q_PROPERTY(QList<ThingId> outdoorSensors READ outdoorSensors)
    Q_PROPERTY(QList<ThingId> notifications READ notifications)
    Q_PROPERTY(ZoneStatus zoneStatus READ zoneStatus)
    Q_PROPERTY(double temperature READ temperature)
    Q_PROPERTY(double humidity READ humidity)
    Q_PROPERTY(uint voc READ voc)
    Q_PROPERTY(double pm25 READ pm25)
    Q_PROPERTY(TemperatureWeekSchedule weekSchedule READ weekSchedule)

public:
    enum ZoneStatusFlag {
        ZoneStatusFlagNone = 0x00,
        ZoneStatusFlagTimeScheduleActive = 0x01,
        ZoneStatusFlagSetpointOverrideActive = 0x02,
        ZoneStatusFlagWindowOpen = 0x10,
        ZoneStatusFlagBadAir = 0x20,
        ZoneStatusFlagHighHumidity = 0x40
    };
    Q_ENUM(ZoneStatusFlag)
    Q_DECLARE_FLAGS(ZoneStatus, ZoneStatusFlag)
//    Q_DECLARE_OPERATORS_FOR_FLAGS(ZoneStatus)
    Q_FLAG(ZoneStatus)

    enum SetpointOverrideMode {
        SetpointOverrideModeNone = 0,
        SetpointOverrideModeTimed,
        SetpointOverrideModeUnlimited,
        SetpointOverrideModeEventual
    };
    Q_ENUM(SetpointOverrideMode)

    ZoneInfo();
    ZoneInfo(const QUuid &id);

    QUuid id() const;

    QString name() const;
    void setName(const QString &name);

    double currentSetpoint() const;
    void setCurrentSetpoint(double currentSetpoint);

    double standbySetpoint() const;
    void setStandbySetpoint(double standbySetpoint);

    double setpointOverride() const;
    void setSetpointOverride(double setpointOverride, SetpointOverrideMode mode, const QDateTime &setpointOverrideEnd = QDateTime());
    SetpointOverrideMode setpointOverrideMode() const;
    QDateTime setpointOverrideEnd() const;

    QList<ThingId> thermostats() const;
    void setThermostats(const QList<ThingId> &thermostats);

    QList<ThingId> windowSensors() const;
    void setWindowSensors(const QList<ThingId> &windowSensors);

    QList<ThingId> indoorSensors() const;
    void setIndoorSensors(const QList<ThingId> &indoorSensors);

    QList<ThingId> outdoorSensors() const;
    void setOutdoorSensors(const QList<ThingId> &outdoorSensors);

    QList<ThingId> notifications() const;
    void setNotifications(const QList<ThingId> &notifications);

    ZoneInfo::ZoneStatus zoneStatus() const;
    void setZoneStatus(ZoneStatus zoneStatus);
    void setZoneStatusFlag(ZoneStatusFlag flag, bool set);

    double temperature() const;
    void setTemperature(double temperature);

    double humidity() const;
    void setHumidity(double humidity);

    uint voc() const;
    void setVoc(uint voc);

    double pm25() const;
    void setPm25(double pm25);

    TemperatureWeekSchedule weekSchedule() const;
    void setWeekSchedule(const TemperatureWeekSchedule &weekSchedule);

private:
    QUuid m_id;
    QString m_name;
    double m_currentSetpoint = 0;
    double m_standbySetpoint = 18;
    double m_setpointOverride;
    SetpointOverrideMode m_setpointOverrideMode = SetpointOverrideModeNone;
    QDateTime m_setpointOverrideEnd;
    QList<ThingId> m_thermostats;
    QList<ThingId> m_windowSensors;
    QList<ThingId> m_indoorSensors;
    QList<ThingId> m_outdoorSensors;
    QList<ThingId> m_notifications;
    ZoneStatus m_zoneStatus = ZoneStatusFlagNone;
    double m_temperature = 0;
    double m_humidity = 0;
    uint m_voc = 0;
    double m_pm25 = 0;
    TemperatureWeekSchedule m_weekSchedule;
};
Q_DECLARE_METATYPE(ZoneInfo)

class ZoneInfos: public QList<ZoneInfo>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    ZoneInfos() = default;
    ZoneInfos(const QList<ZoneInfo> &other): QList<ZoneInfo>(other) {}
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};
Q_DECLARE_METATYPE(QList<ZoneInfo>)
Q_DECLARE_METATYPE(ZoneInfos)

#endif // ZONEINFO_H
