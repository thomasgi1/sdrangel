#ifndef INCLUDE_FEATURE_FREQDISPLAYSETTINGS_H_
#define INCLUDE_FEATURE_FREQDISPLAYSETTINGS_H_

#include <QByteArray>
#include <QString>
#include <QStringList>

struct FreqDisplaySettings
{
    QString m_title;
    QString m_selectedChannel;
    int m_workspaceIndex;
    QByteArray m_geometryBytes;

    FreqDisplaySettings();
    ~FreqDisplaySettings() = default;

    void resetToDefaults();
    QByteArray serialize() const;
    bool deserialize(const QByteArray& data);
    void applySettings(const QStringList& settingsKeys, const FreqDisplaySettings& settings);
};

#endif // INCLUDE_FEATURE_FREQDISPLAYSETTINGS_H_
