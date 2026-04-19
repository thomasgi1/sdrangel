///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2026 Jon Beniston, M7RCE <jon@beniston.com>                     //
// Some code by Copilot                                                          //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
// (at your option) any later version.                                           //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////

#include <QDebug>

#include "freqdisplay.h"

const char* const FreqDisplay::m_featureIdURI = "sdrangel.feature.freqdisplay";
const char* const FreqDisplay::m_featureId = "FreqDisplay";

FreqDisplay::FreqDisplay(WebAPIAdapterInterface *webAPIAdapterInterface) :
    Feature(m_featureIdURI, webAPIAdapterInterface)
{
    qDebug("FreqDisplay::FreqDisplay: webAPIAdapterInterface: %p", webAPIAdapterInterface);
    setObjectName(m_featureId);
    m_state = StIdle;
}

bool FreqDisplay::handleMessage(const Message& cmd)
{
    (void) cmd;
    return false;
}

QByteArray FreqDisplay::serialize() const
{
    return m_settings.serialize();
}

bool FreqDisplay::deserialize(const QByteArray& data)
{
    if (!m_settings.deserialize(data)) {
        m_settings.resetToDefaults();
        return false;
    }

    return true;
}

void FreqDisplay::applySettings(const FreqDisplaySettings& settings, const QStringList& settingsKeys, bool force)
{
    if (force) {
        m_settings = settings;
    } else {
        m_settings.applySettings(settingsKeys, settings);
    }
}
