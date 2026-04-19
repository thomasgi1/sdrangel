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

#include <QColor>

#include "util/simpleserializer.h"
#include "settings/serializable.h"

#include "freqdisplaysettings.h"

FreqDisplaySettings::FreqDisplaySettings() :
    m_rollupState(nullptr)
{
    resetToDefaults();
}

void FreqDisplaySettings::resetToDefaults()
{
    m_title = "Frequency Display";
    m_selectedChannel.clear();
    m_workspaceIndex = -1;
    m_geometryBytes.clear();
    m_fontName.clear();
    m_transparentBackground = false;
    m_displayMode = Frequency;
    m_speechEnabled = false;
    m_frequencyUnits = Hz;
    m_showUnits = true;
    m_freqDecimalPlaces = 3;
    m_powerDecimalPlaces = 1;
    m_textColor = Qt::white;
}

QByteArray FreqDisplaySettings::serialize() const
{
    SimpleSerializer s(1);

    s.writeString(1, m_title);
    s.writeString(2, m_selectedChannel);
    s.writeS32(3, m_workspaceIndex);
    s.writeBlob(4, m_geometryBytes);
    s.writeString(5, m_fontName);
    s.writeBool(6, m_transparentBackground);
    s.writeS32(7, static_cast<int>(m_displayMode));
    s.writeBool(8, m_speechEnabled);
    s.writeS32(9, static_cast<int>(m_frequencyUnits));
    s.writeBool(10, m_showUnits);
    s.writeS32(11, m_freqDecimalPlaces);
    s.writeS32(12, m_powerDecimalPlaces);
    s.writeU32(13, m_textColor.rgba());
    if (m_rollupState) {
        s.writeBlob(14, m_rollupState->serialize());
    }

    return s.final();
}

bool FreqDisplaySettings::deserialize(const QByteArray& data)
{
    SimpleDeserializer d(data);
    QByteArray bytetmp;

    if (!d.isValid())
    {
        resetToDefaults();
        return false;
    }

    if (d.getVersion() != 1)
    {
        resetToDefaults();
        return false;
    }

    d.readString(1, &m_title, "Frequency Display");
    d.readString(2, &m_selectedChannel, "");
    d.readS32(3, &m_workspaceIndex, -1);
    d.readBlob(4, &m_geometryBytes);
    d.readString(5, &m_fontName, "");
    d.readBool(6, &m_transparentBackground, false);
    int displayMode = 0;
    d.readS32(7, &displayMode, 0);
    m_displayMode = static_cast<DisplayMode>(displayMode);
    d.readBool(8, &m_speechEnabled, false);
    int frequencyUnits = 0;
    d.readS32(9, &frequencyUnits, 0);
    m_frequencyUnits = static_cast<FrequencyUnits>(frequencyUnits);
    d.readBool(10, &m_showUnits, true);
    d.readS32(11, &m_freqDecimalPlaces, 3);
    d.readS32(12, &m_powerDecimalPlaces, 1);
    quint32 rgba = QColor(Qt::white).rgba();
    d.readU32(13, &rgba, QColor(Qt::white).rgba());
    m_textColor = QColor::fromRgba(rgba);
    if (m_rollupState)
    {
        d.readBlob(14, &bytetmp);
        m_rollupState->deserialize(bytetmp);
    }

    return true;
}

void FreqDisplaySettings::applySettings(const QStringList& settingsKeys, const FreqDisplaySettings& settings)
{
    if (settingsKeys.contains("title")) {
        m_title = settings.m_title;
    }
    if (settingsKeys.contains("selectedChannel")) {
        m_selectedChannel = settings.m_selectedChannel;
    }
    if (settingsKeys.contains("workspaceIndex")) {
        m_workspaceIndex = settings.m_workspaceIndex;
    }
    if (settingsKeys.contains("geometryBytes")) {
        m_geometryBytes = settings.m_geometryBytes;
    }
    if (settingsKeys.contains("fontName")) {
        m_fontName = settings.m_fontName;
    }
    if (settingsKeys.contains("transparentBackground")) {
        m_transparentBackground = settings.m_transparentBackground;
    }
    if (settingsKeys.contains("displayMode")) {
        m_displayMode = settings.m_displayMode;
    }
    if (settingsKeys.contains("speechEnabled")) {
        m_speechEnabled = settings.m_speechEnabled;
    }
    if (settingsKeys.contains("frequencyUnits")) {
        m_frequencyUnits = settings.m_frequencyUnits;
    }
    if (settingsKeys.contains("showUnits")) {
        m_showUnits = settings.m_showUnits;
    }
    if (settingsKeys.contains("freqDecimalPlaces")) {
        m_freqDecimalPlaces = settings.m_freqDecimalPlaces;
    }
    if (settingsKeys.contains("powerDecimalPlaces")) {
        m_powerDecimalPlaces = settings.m_powerDecimalPlaces;
    }
    if (settingsKeys.contains("textColor")) {
        m_textColor = settings.m_textColor;
    }
}
