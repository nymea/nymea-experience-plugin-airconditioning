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

#include "thermostat.h"

#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(dcAirConditioning)

Thermostat::Thermostat(ThingManager *thingManager, Thing *thing, QObject *parent):
    QObject(parent),
    m_thingManager(thingManager),
    m_thing(thing)
{
    m_cachedTargetTemperature = m_thing->stateValue("targetTemperature").toDouble();
}

Thing *Thermostat::thing() const
{
    return m_thing;
}

void Thermostat::setTargetTemperature(double targetTemperature, bool force)
{
    qCDebug(dcAirConditioning()) << "setTargetTemp called. Window open:" << m_windowOpen << "force:" << force;
    m_cachedTargetTemperature = targetTemperature;
    if (m_windowOpen && !force) {
        qCDebug(dcAirConditioning()) << "Not setting target temperature on" << m_thing->name() << "because a window is open";
        return;
    }

    if (m_thing->stateValue("targetTemperature").toDouble() != targetTemperature) {
        ActionType actionType = m_thing->thingClass().actionTypes().findByName("targetTemperature");
        Action action(actionType.id(), m_thing->id(), Action::TriggeredByRule);
        action.setParams({Param(actionType.id(), targetTemperature)});
        qCDebug(dcAirConditioning()) << "Setting target temperature" << targetTemperature << "to" << m_thing->name() << "from" << m_thing->stateValue("targetTemperature").toDouble();
        ThingActionInfo *info = m_thingManager->executeAction(action);
        connect(info, &ThingActionInfo::finished, this, [info, this](){
            if (info->status() != Thing::ThingErrorNoError) {
                qCWarning(dcAirConditioning()) << "Unable to execute targetTemperature action on" << m_thing << info->status() << info->displayMessage();
                return;
            }
            qCDebug(dcAirConditioning()) << "Target temperature set successfully";
        });
    }
}

void Thermostat::setWindowOpen(bool windowOpen)
{
    m_windowOpen = windowOpen;

    // First check if the device is capable of handling a window open locks
    if (!m_thing->thingClass().actionTypes().findByName("windowOpen").id().isNull()) {
        if (m_thing->stateValue("windowOpen").toBool() != windowOpen) {
            ActionType actionType = m_thing->thingClass().actionTypes().findByName("windowOpen");
            Action action(actionType.id(), m_thing->id(), Action::TriggeredByRule);
            action.setParams({Param(actionType.id(), windowOpen)});
            qCDebug(dcAirConditioning()) << "Setting window open" << windowOpen << "to" << m_thing->name();
            ThingActionInfo *info = m_thingManager->executeAction(action);
            connect(info, &ThingActionInfo::finished, this, [info, this](){
                if (info->status() != Thing::ThingErrorNoError) {
                    qCWarning(dcAirConditioning()) << "Unable to execute window Open action on" << m_thing << info->status() << info->displayMessage();
                    return;
                }
            });
        }
        return;
    }

    // Otherwise see if it can be turned off while the window is open
    if (m_thing->hasState("power")) {
        if (m_thing->stateValue("power").toBool() == windowOpen) {
            ActionType actionType = m_thing->thingClass().actionTypes().findByName("power");
            Action action(actionType.id(), m_thing->id(), Action::TriggeredByRule);
            action.setParams({Param(actionType.id(), !windowOpen)});
            qCDebug(dcAirConditioning()) << "Setting power" << !windowOpen << "to" << m_thing->name();
            ThingActionInfo *info = m_thingManager->executeAction(action);
            connect(info, &ThingActionInfo::finished, this, [info, this](){
                if (info->status() != Thing::ThingErrorNoError) {
                    qCWarning(dcAirConditioning()) << "Unable to execute power action on" << m_thing << info->status() << info->displayMessage();
                    return;
                }
            });
        }
    }

    // If nothing works, let's assume it is a very dump radiator thermostat and set the temperature to minimum
    double temp = windowOpen ? m_thing->state("targetTemperature").minValue().toDouble() : m_cachedTargetTemperature;
    if (m_thing->stateValue("targetTemperature").toDouble() != temp) {
        ActionType actionType = m_thing->thingClass().actionTypes().findByName("targetTemperature");
        Action action(actionType.id(), m_thing->id(), Action::TriggeredByRule);
        action.setParams({Param(actionType.id(), temp)});
        qCDebug(dcAirConditioning()) << "Setting target temperature (window open control)" << temp << "to" << m_thing->name() << "from" << m_thing->stateValue("targetTemperature").toDouble();
        ThingActionInfo *info = m_thingManager->executeAction(action);
        connect(info, &ThingActionInfo::finished, this, [info, this](){
            if (info->status() != Thing::ThingErrorNoError) {
                qCWarning(dcAirConditioning()) << "Unable to execute targetTemperature action on" << m_thing << info->status() << info->displayMessage();
                return;
            }
            qCDebug(dcAirConditioning()) << "Target temperature set successfully";
        });
    }
}

bool Thermostat::hasTemperatureSensor() const
{
    return m_thing->thingClass().interfaces().contains("temperaturesensor");
}

double Thermostat::temperature() const
{
    return m_thing->stateValue("temperature").toDouble();
}

