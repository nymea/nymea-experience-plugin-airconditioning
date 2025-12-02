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

#include "temperatureschedule.h"

TemperatureSchedule::TemperatureSchedule()
{

}

TemperatureSchedule::TemperatureSchedule(const QTime &startTime, const QTime &endTime, double temperature):
    m_startTime(startTime),
    m_endTime(endTime),
    m_temperature(temperature)
{

}

QTime TemperatureSchedule::startTime() const
{
    return m_startTime;
}

void TemperatureSchedule::setStartTime(const QTime &startTime)
{
    m_startTime = startTime;
}

QTime TemperatureSchedule::endTime() const
{
    return m_endTime;
}

void TemperatureSchedule::setEndTime(const QTime &endTime)
{
    m_endTime = endTime;
}

double TemperatureSchedule::temperature() const
{
    return m_temperature;
}

void TemperatureSchedule::setTemperature(double temperature)
{
    m_temperature = temperature;
}

TemperatureDaySchedule::TemperatureDaySchedule(const QList<TemperatureSchedule> &other):
    QList<TemperatureSchedule>(other)
{

}

QVariant TemperatureDaySchedule::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void TemperatureDaySchedule::put(const QVariant &variant)
{
    append(variant.value<TemperatureSchedule>());
}

TemperatureWeekSchedule TemperatureWeekSchedule::create() {
    TemperatureWeekSchedule ret;
    for (int day = 0; day < 7; day++) {
        ret.append(TemperatureDaySchedule());
    }
    return ret;
}

QVariant TemperatureWeekSchedule::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void TemperatureWeekSchedule::put(const QVariant &variant)
{
    append(variant.value<TemperatureDaySchedule>());
}

QDebug operator<<(QDebug dbg, const TemperatureSchedule &schedule)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace().noquote() << schedule.startTime().toString("hh:mm") << " - " << schedule.endTime().toString("hh:mm") << ": " << schedule.temperature();
    return dbg;
}

QDebug operator<<(QDebug dbg, const TemperatureDaySchedule &daySchedule)
{
    for (int i = 0; i < daySchedule.count(); i++) {
        dbg << daySchedule.at(i);
        if (i < daySchedule.count() - 1) {
            QDebugStateSaver saver(dbg);
            dbg.nospace() << ", ";
        }
    }
    return dbg;
}

QDebug operator<<(QDebug dbg, const TemperatureWeekSchedule &weekSchedule)
{
    QDebugStateSaver saver(dbg);
    QStringList days = { "Mo", "Tu", "We", "Th", "Fr", "Sa", "Su" };
    dbg << "\n";
    for (int i = 0; i < qMin(weekSchedule.count(), 7); i++) {
        dbg.nospace().noquote() << days.at(i) << ": " << weekSchedule.at(i) << "\n";
    }
    return dbg;
}

