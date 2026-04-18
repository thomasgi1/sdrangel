#include "util/simpleserializer.h"

#include "freqdisplaysettings.h"

FreqDisplaySettings::FreqDisplaySettings()
{
    resetToDefaults();
}

void FreqDisplaySettings::resetToDefaults()
{
    m_title = "Frequency display";
    m_selectedChannel.clear();
    m_workspaceIndex = -1;
    m_geometryBytes.clear();
}

QByteArray FreqDisplaySettings::serialize() const
{
    SimpleSerializer s(1);

    s.writeString(1, m_title);
    s.writeString(2, m_selectedChannel);
    s.writeS32(3, m_workspaceIndex);
    s.writeBlob(4, m_geometryBytes);

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

    d.readString(1, &m_title, "Frequency display");
    d.readString(2, &m_selectedChannel, "");
    d.readS32(3, &m_workspaceIndex, -1);
    d.readBlob(4, &m_geometryBytes);

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
}
