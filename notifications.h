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

#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <QObject>
#include <QTimer>

#include <integrations/thing.h>
#include <integrations/thingmanager.h>

#include "zoneinfo.h"


class Notifications : public QObject
{
    Q_OBJECT
public:
    explicit Notifications(ThingManager *thingManager, Thing *thing, QObject *parent = nullptr);

    void update(const ZoneInfo &zone);
signals:

private:
    ThingActionInfo *updateNotification(const QString &id, const QString &title, const QString &text, bool sound, bool remove);
private:
    ThingManager *m_thingManager = nullptr;
    Thing *m_thing = nullptr;

    ZoneInfo::ZoneStatus m_zoneStatus;

    bool m_humidityWarningShown = false;
    double m_lastHumidityValue = 0;
    bool m_badAirWarningShown = false;
    uint m_lastBadAirValue = 0;

    // For devices that don't support updates/removals, we'll assume after some time that it's gone and we may need to show again
    QTimer m_clearHumidityTimer;
    QTimer m_clearBadAirTimer;
};


#endif // NOTIFICATIONS_H
