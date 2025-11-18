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

#ifndef AIRCONDITIONINGJSONHANDLER_H
#define AIRCONDITIONINGJSONHANDLER_H

#include <QObject>

#include <jsonrpc/jsonhandler.h>

class AirConditioningManager;

class AirConditioningJsonHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit AirConditioningJsonHandler(AirConditioningManager *manager, QObject *parent = nullptr);

    QString name() const override;

    Q_INVOKABLE JsonReply *GetZones(const QVariantMap &params);
    Q_INVOKABLE JsonReply *AddZone(const QVariantMap &params);
    Q_INVOKABLE JsonReply *RemoveZone(const QVariantMap &params);
    Q_INVOKABLE JsonReply *SetZoneName(const QVariantMap &params);
    Q_INVOKABLE JsonReply *SetZoneStandbySetpoint(const QVariantMap &params);
    Q_INVOKABLE JsonReply *SetZoneSetpointOverride(const QVariantMap &params);
    Q_INVOKABLE JsonReply* SetZoneWeekSchedule(const QVariantMap &params);
    Q_INVOKABLE JsonReply *SetZoneThings(const QVariantMap &params);

signals:
    void ZoneAdded(const QVariantMap &params);
    void ZoneRemoved(const QVariantMap &params);
    void ZoneChanged(const QVariantMap &params);

private:
    AirConditioningManager *m_manager = nullptr;
};

#endif // AIRCONDITIONINGJSONHANDLER_H
