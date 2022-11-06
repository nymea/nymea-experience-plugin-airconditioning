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

#ifndef TEMPERATURESCHEDULE_H
#define TEMPERATURESCHEDULE_H

#include <QObject>
#include <QTime>
#include <QVariant>
#include <QDebug>

class TemperatureSchedule
{
    Q_GADGET

    Q_PROPERTY(QTime startTime READ startTime WRITE setStartTime)
    Q_PROPERTY(QTime endTime READ endTime WRITE setEndTime)
    Q_PROPERTY(double temperature READ temperature WRITE setTemperature)

public:
    TemperatureSchedule();
    TemperatureSchedule(const QTime &startTime, const QTime &endTime, double temperature);

    QTime startTime() const;
    void setStartTime(const QTime &startTime);

    QTime endTime() const;
    void setEndTime(const QTime &endTime);

    double temperature() const;
    void setTemperature(double temperature);

private:
    QTime m_startTime;
    QTime m_endTime;
    double m_temperature = 0;
};
Q_DECLARE_METATYPE(TemperatureSchedule)

class TemperatureDaySchedule: public QList<TemperatureSchedule>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    TemperatureDaySchedule() = default;
    TemperatureDaySchedule(const QList<TemperatureSchedule> &other);
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};
Q_DECLARE_METATYPE(QList<TemperatureSchedule>)
Q_DECLARE_METATYPE(TemperatureDaySchedule)

class TemperatureWeekSchedule: public QList<TemperatureDaySchedule>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)
public:
    static TemperatureWeekSchedule create();
    TemperatureWeekSchedule() = default;
    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};
Q_DECLARE_METATYPE(QList<TemperatureDaySchedule>)
Q_DECLARE_METATYPE(TemperatureWeekSchedule)

//class TemperatureWeekSchedules
//{
//    Q_GADGET
//    Q_PROPERTY(TemperatureSchedules monday READ monday WRITE setMonday)
//    Q_PROPERTY(TemperatureSchedules tuesday READ tuesday WRITE setTuesday)
//    Q_PROPERTY(TemperatureSchedules wednesday READ wednesday WRITE setWednesday)
//    Q_PROPERTY(TemperatureSchedules thursday READ thursday WRITE setThursday)
//    Q_PROPERTY(TemperatureSchedules friday READ friday WRITE setFriday)
//    Q_PROPERTY(TemperatureSchedules saturday READ saturday WRITE setSaturday)
//    Q_PROPERTY(TemperatureSchedules sunday READ sunday WRITE setSunday)

//public:
//    TemperatureWeekSchedules() = default;

//    TemperatureSchedules monday() const { return m_monday; }
//    void setMonday(const TemperatureSchedules &monday) { m_monday = monday; }

//    TemperatureSchedules tuesday() const { return m_tuesday; }
//    void setTuesday(const TemperatureSchedules &tuesday) { m_tuesday = tuesday; }

//    TemperatureSchedules wednesday() const { return m_wednesday; }
//    void setWednesday(const TemperatureSchedules &wednesday) { m_wednesday = wednesday; }

//    TemperatureSchedules thursday() const { return m_thursday; }
//    void setThursday(const TemperatureSchedules &thursday) { m_thursday = thursday; }

//    TemperatureSchedules friday() const { return m_friday; }
//    void setFriday(const TemperatureSchedules &friday) { m_friday = friday; }

//    TemperatureSchedules saturday() const { return m_saturday; }
//    void setSaturday(const TemperatureSchedules &satuday) { m_saturday = satuday; }

//    TemperatureSchedules sunday() const { return m_sunday; }
//    void setSunday(const TemperatureSchedules &sunday) { m_sunday = sunday; }

//private:
//    TemperatureSchedules m_monday;
//    TemperatureSchedules m_tuesday;
//    TemperatureSchedules m_wednesday;
//    TemperatureSchedules m_thursday;
//    TemperatureSchedules m_friday;
//    TemperatureSchedules m_saturday;
//    TemperatureSchedules m_sunday;

//};
//Q_DECLARE_METATYPE(TemperatureWeekSchedules)


QDebug operator<<(QDebug debug, const TemperatureSchedule &schedule);
QDebug operator<<(QDebug debug, const TemperatureDaySchedule &daySchedule);
QDebug operator<<(QDebug debug, const TemperatureWeekSchedule &weekSchedules);

#endif // TEMPERATURESCHEDULE_H
