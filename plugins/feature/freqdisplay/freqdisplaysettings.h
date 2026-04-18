#ifndef INCLUDE_FEATURE_FREQDISPLAYSETTINGS_H_
#define INCLUDE_FEATURE_FREQDISPLAYSETTINGS_H_

#include <QByteArray>
#include <QString>
#include <QStringList>

struct FreqDisplaySettings
{
    enum DisplayMode {
        Frequency = 0, //!< Show channel centre frequency
        Power     = 1, //!< Show channel power in dBFS
        Both      = 2  //!< Show frequency and power on two lines
    };

    QString m_title;
    QString m_selectedChannel;
    int m_workspaceIndex;
    QByteArray m_geometryBytes;
    QString m_fontName;
    bool m_transparentBackground;
    DisplayMode m_displayMode;
    bool m_speechEnabled;

    FreqDisplaySettings();
    ~FreqDisplaySettings() = default;

    void resetToDefaults();
    QByteArray serialize() const;
    bool deserialize(const QByteArray& data);
    void applySettings(const QStringList& settingsKeys, const FreqDisplaySettings& settings);
};

#endif // INCLUDE_FEATURE_FREQDISPLAYSETTINGS_H_
