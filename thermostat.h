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

#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include <QObject>

#include <integrations/thing.h>
#include <integrations/thingmanager.h>

class Thermostat : public QObject
{
    Q_OBJECT
public:
    explicit Thermostat(ThingManager *thingManager, Thing *thing, QObject *parent = nullptr);

    Thing *thing() const;
    void setTargetTemperature(double targetTemperature, bool force = false);
    void setWindowOpen(bool windowOpen);

    bool hasTemperatureSensor() const;
    double temperature() const;

signals:

private:
    ThingManager *m_thingManager = nullptr;
    Thing *m_thing = nullptr;

    double m_cachedTargetTemperature;
    bool m_windowOpen = false;
};

#endif // THERMOSTAT_H
