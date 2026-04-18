#ifndef INCLUDE_FEATURE_FREQDISPLAYGUI_H_
#define INCLUDE_FEATURE_FREQDISPLAYGUI_H_

#include <QTimer>

#include "availablechannelorfeaturehandler.h"
#include "feature/featuregui.h"
#include "util/messagequeue.h"

#include "freqdisplaysettings.h"

class PluginAPI;
class FeatureUISet;
class FreqDisplay;
class Feature;

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

    explicit FreqDisplayGUI(PluginAPI* pluginAPI, FeatureUISet *featureUISet, Feature *feature, QWidget* parent = nullptr);
    ~FreqDisplayGUI() override;

    void displaySettings();
    void applySettings(bool force = false);
    void updateChannelList();
    void updateFrequencyText();
    void updateFrequencyFont();

private slots:
    void channelsOrFeaturesChanged(const QStringList& renameFrom, const QStringList& renameTo, const QStringList& removed, const QStringList& added);
    void on_channels_currentIndexChanged(int index);
    void pollSelectedChannel();
};

#endif // INCLUDE_FEATURE_FREQDISPLAYGUI_H_
