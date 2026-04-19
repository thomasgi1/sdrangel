#ifndef INCLUDE_FEATURE_FREQDISPLAYGUI_H_
#define INCLUDE_FEATURE_FREQDISPLAYGUI_H_

#include <QTimer>
#include <QFontComboBox>

#ifdef QT_TEXTTOSPEECH_FOUND
#include <QTextToSpeech>
#endif

#include "availablechannelorfeaturehandler.h"
#include "feature/featuregui.h"
#include "util/messagequeue.h"

#include "freqdisplaysettings.h"

class PluginAPI;
class FeatureUISet;
class FreqDisplay;
class Feature;
class QMdiArea;

namespace Ui {
class FreqDisplayGUI;
}

class FreqDisplayGUI : public FeatureGUI
{
    Q_OBJECT

public:
    static FreqDisplayGUI* create(PluginAPI* pluginAPI, FeatureUISet *featureUISet, Feature *feature);
    void destroy() override;

    void resetToDefaults() override;
    QByteArray serialize() const override;
    bool deserialize(const QByteArray& data) override;
    MessageQueue *getInputMessageQueue() override { return &m_inputMessageQueue; }
    void setWorkspaceIndex(int index) override;
    int getWorkspaceIndex() const override { return m_settings.m_workspaceIndex; }
    void setGeometryBytes(const QByteArray& blob) override { m_settings.m_geometryBytes = blob; }
    QByteArray getGeometryBytes() const override { return m_settings.m_geometryBytes; }

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::FreqDisplayGUI* ui;
    FreqDisplay* m_freqDisplay;
    FreqDisplaySettings m_settings;
    MessageQueue m_inputMessageQueue;
    AvailableChannelOrFeatureHandler m_availableChannelOrFeatureHandler;
    AvailableChannelOrFeatureList m_availableChannels;
    QTimer m_pollTimer;
    bool m_doApplySettings;
    QString m_normalStyleSheet; ///< Stylesheet set by FeatureGUI, saved so it can be restored when transparency is disabled
    QString m_previousDisplayText; ///< Last text set on frequencyValue, used to detect changes for speech
    QMdiArea* m_savedMdi;   ///< MDI area saved when window is detached for transparent-background mode
    QRect m_mdiGeometry;    ///< Window geometry (in MDI viewport coordinates) saved before detaching

#ifdef QT_TEXTTOSPEECH_FOUND
    QTextToSpeech *m_speech = nullptr;
    QString m_pendingSpeechText; ///< Most recent text to speak once the engine is no longer busy
#endif

    explicit FreqDisplayGUI(PluginAPI* pluginAPI, FeatureUISet *featureUISet, Feature *feature, QWidget* parent = nullptr);
    ~FreqDisplayGUI() override;

    void displaySettings();
    void applySettings(bool force = false);
    void updateChannelList();
    void updateFrequencyText();
    void updateFrequencyFont();
    void updateFreqDecimalSpinbox();
    void applyTransparency();
    void applySpeech();
    QString formatFrequency(qint64 frequencyHz) const;

private slots:
    void channelsOrFeaturesChanged(const QStringList& renameFrom, const QStringList& renameTo, const QStringList& removed, const QStringList& added);
    void on_channels_currentIndexChanged(int index);
    void on_displayMode_currentIndexChanged(int index);
    void on_speech_toggled(bool checked);
    void on_fontFamily_currentFontChanged(const QFont& font);
    void on_transparentBackground_toggled(bool checked);
    void on_frequencyUnits_currentIndexChanged(int index);
    void on_showUnits_toggled(bool checked);
    void on_freqDecimalPlaces_valueChanged(int value);
    void on_powerDecimalPlaces_valueChanged(int value);
    void pollSelectedChannel();
#ifdef QT_TEXTTOSPEECH_FOUND
    void speechStateChanged(QTextToSpeech::State state);
#endif
};

#endif // INCLUDE_FEATURE_FREQDISPLAYGUI_H_
