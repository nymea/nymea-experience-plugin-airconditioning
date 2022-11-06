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

#include "zoneinfo.h"


ZoneInfo::ZoneInfo()
{

}

ZoneInfo::ZoneInfo(const QUuid &id):
    m_id(id)
{

}

QUuid ZoneInfo::id() const
{
    return m_id;
}

QString ZoneInfo::name() const
{
    return m_name;
}

void ZoneInfo::setName(const QString &name)
{
    m_name = name;
}

double ZoneInfo::currentSetpoint() const
{
    return m_currentSetpoint;
}

void ZoneInfo::setCurrentSetpoint(double currentSetpoint)
{
    m_currentSetpoint = currentSetpoint;
}

double ZoneInfo::standbySetpoint() const
{
    return m_standbySetpoint;
}

void ZoneInfo::setStandbySetpoint(double standbySetpoint)
{
    m_standbySetpoint = standbySetpoint;
}

double ZoneInfo::setpointOverride() const
{
    return m_setpointOverride;
}

void ZoneInfo::setSetpointOverride(double setpointOverride, SetpointOverrideMode mode, const QDateTime &setpointOverrideEnd)
{
    m_setpointOverride = setpointOverride;
    m_setpointOverrideMode = mode;
    m_setpointOverrideEnd = setpointOverrideEnd;
}

ZoneInfo::SetpointOverrideMode ZoneInfo::setpointOverrideMode() const
{
    return m_setpointOverrideMode;
}

QDateTime ZoneInfo::setpointOverrideEnd() const
{
    return m_setpointOverrideEnd;
}

QList<ThingId> ZoneInfo::thermostats() const
{
    return m_thermostats;
}

void ZoneInfo::setThermostats(const QList<ThingId> &thermostats)
{
    m_thermostats = thermostats;
}

QList<ThingId> ZoneInfo::windowSensors() const
{
    return m_windowSensors;
}

void ZoneInfo::setWindowSensors(const QList<ThingId> &windowSensors)
{
    m_windowSensors = windowSensors;
}

QList<ThingId> ZoneInfo::indoorSensors() const
{
    return m_indoorSensors;
}

void ZoneInfo::setIndoorSensors(const QList<ThingId> &indoorSensors)
{
    m_indoorSensors = indoorSensors;
}

QList<ThingId> ZoneInfo::outdoorSensors() const
{
    return m_outdoorSensors;
}

void ZoneInfo::setOutdoorSensors(const QList<ThingId> &outdoorSensors)
{
    m_outdoorSensors = outdoorSensors;
}

ZoneInfo::ZoneStatus ZoneInfo::zoneStatus() const
{
    return m_zoneStatus;
}

void ZoneInfo::setZoneStatus(ZoneStatus zoneStatus)
{
    m_zoneStatus = zoneStatus;
}

void ZoneInfo::setZoneStatusFlag(ZoneStatusFlag flag, bool set)
{
    m_zoneStatus.setFlag(flag, set);
}

TemperatureWeekSchedule ZoneInfo::weekSchedule() const
{
    return m_weekSchedule;
}

void ZoneInfo::setWeekSchedule(const TemperatureWeekSchedule &weekSchedule)
{
    m_weekSchedule = weekSchedule;
    while (m_weekSchedule.count() < 7) {
        m_weekSchedule.append(TemperatureDaySchedule());
    }
}

QVariant ZoneInfos::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void ZoneInfos::put(const QVariant &variant)
{
    append(variant.value<ZoneInfo>());
}
