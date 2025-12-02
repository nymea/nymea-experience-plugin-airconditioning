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

#ifndef EXPERIENCEPLUGINAIRCONDITIONING_H
#define EXPERIENCEPLUGINAIRCONDITIONING_H

#include <experiences/experienceplugin.h>

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(dcAirConditioning)

class AirConditioningManager;

class ExperiencePluginAirConditioning: public ExperiencePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "io.nymea.ExperiencePlugin")
    Q_INTERFACES(ExperiencePlugin)

public:
    ExperiencePluginAirConditioning();
    void init() override;

private:

    AirConditioningManager *m_manager = nullptr;
};

#endif // EXPERIENCEPLUGINAIRCONDITIONING_H
