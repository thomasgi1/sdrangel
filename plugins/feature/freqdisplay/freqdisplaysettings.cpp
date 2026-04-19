#include "util/simpleserializer.h"

#include "freqdisplaysettings.h"

FreqDisplaySettings::FreqDisplaySettings()
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

    return s.final();
}

bool FreqDisplaySettings::deserialize(const QByteArray& data)
{
    SimpleDeserializer d(data);

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
}
