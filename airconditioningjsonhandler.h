/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2025, nymea GmbH
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
