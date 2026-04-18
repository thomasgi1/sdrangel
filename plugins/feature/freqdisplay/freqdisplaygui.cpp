#include <QFont>
#include <QLocale>
#include <QResizeEvent>

#include "channel/channelwebapiutils.h"

#include "feature/featureuiset.h"

#include "ui_freqdisplaygui.h"
#include "freqdisplay.h"
#include "freqdisplaygui.h"

namespace {
// For typical feature windows this keeps the text close to ~22% of the smallest
// widget dimension, which yields large readable digits without clipping.
constexpr double frequencyFontScale = 0.22;
constexpr const char* rxTxChannelKinds = "RT";
constexpr int pollIntervalMs = 1000;
constexpr int minimumFrequencyFontPointSize = 10;
}

FreqDisplayGUI* FreqDisplayGUI::create(PluginAPI* pluginAPI, FeatureUISet *featureUISet, Feature *feature)
{
    return new FreqDisplayGUI(pluginAPI, featureUISet, feature);
}

void FreqDisplayGUI::destroy()
{
    delete this;
}

void FreqDisplayGUI::resetToDefaults()
{
    m_settings.resetToDefaults();
    displaySettings();
    applySettings(true);
    updateFrequencyText();
}

QByteArray FreqDisplayGUI::serialize() const
{
    return m_settings.serialize();
}

bool FreqDisplayGUI::deserialize(const QByteArray& data)
{
    if (m_settings.deserialize(data))
    {
        m_feature->setWorkspaceIndex(m_settings.m_workspaceIndex);
        displaySettings();
        applySettings(true);
        updateFrequencyText();
        return true;
    }

    resetToDefaults();
    return false;
}

FreqDisplayGUI::FreqDisplayGUI(PluginAPI* pluginAPI, FeatureUISet *featureUISet, Feature *feature, QWidget* parent) :
    FeatureGUI(parent),
    ui(new Ui::FreqDisplayGUI),
    m_freqDisplay(reinterpret_cast<FreqDisplay*>(feature)),
    m_availableChannelOrFeatureHandler(QStringList(), rxTxChannelKinds),
    m_doApplySettings(true)
{
    (void) pluginAPI;
    (void) featureUISet;

    m_feature = feature;
    setAttribute(Qt::WA_DeleteOnClose, true);

    RollupContents *rollupContents = getRollupContents();
    ui->setupUi(rollupContents);

    m_freqDisplay->setMessageQueueToGUI(&m_inputMessageQueue);
    m_settings = m_freqDisplay->getSettings();

    connect(
        &m_availableChannelOrFeatureHandler,
        &AvailableChannelOrFeatureHandler::channelsOrFeaturesChanged,
        this,
        &FreqDisplayGUI::channelsOrFeaturesChanged
    );
    m_availableChannelOrFeatureHandler.scanAvailableChannelsAndFeatures();

    connect(ui->channels, qOverload<int>(&QComboBox::currentIndexChanged), this, &FreqDisplayGUI::on_channels_currentIndexChanged);
    connect(&m_pollTimer, &QTimer::timeout, this, &FreqDisplayGUI::pollSelectedChannel);
    m_pollTimer.start(pollIntervalMs);

    displaySettings();
    updateFrequencyText();
}

FreqDisplayGUI::~FreqDisplayGUI()
{
    delete ui;
}

void FreqDisplayGUI::setWorkspaceIndex(int index)
{
    m_settings.m_workspaceIndex = index;
    m_feature->setWorkspaceIndex(index);
}

void FreqDisplayGUI::displaySettings()
{
    setWindowTitle(m_settings.m_title);
    setTitle(m_settings.m_title);

    updateChannelList();
}

void FreqDisplayGUI::applySettings(bool force)
{
    if (!m_doApplySettings) {
        return;
    }

    QStringList settingsKeys;
    settingsKeys.append("title");
    settingsKeys.append("selectedChannel");
    settingsKeys.append("workspaceIndex");
    settingsKeys.append("geometryBytes");
    m_freqDisplay->applySettings(m_settings, settingsKeys, force);
}

void FreqDisplayGUI::updateChannelList()
{
    m_availableChannels = m_availableChannelOrFeatureHandler.getAvailableChannelOrFeatureList();

    ui->channels->blockSignals(true);
    ui->channels->clear();

    int selectedIndex = -1;

    for (int i = 0; i < m_availableChannels.size(); ++i)
    {
        const QString longId = m_availableChannels.at(i).getLongId();
        ui->channels->addItem(longId);

        if (longId == m_settings.m_selectedChannel) {
            selectedIndex = i;
        }
    }

    if ((selectedIndex < 0) && (m_availableChannels.size() > 0)) {
        selectedIndex = 0;
    }

    if (selectedIndex >= 0)
    {
        ui->channels->setCurrentIndex(selectedIndex);
        m_settings.m_selectedChannel = m_availableChannels.at(selectedIndex).getLongId();
    }
    else
    {
        m_settings.m_selectedChannel.clear();
    }

    ui->channels->blockSignals(false);
}

void FreqDisplayGUI::channelsOrFeaturesChanged(const QStringList& renameFrom, const QStringList& renameTo, const QStringList& removed, const QStringList& added)
{
    (void) removed;
    (void) added;

    for (int i = 0; i < renameFrom.size() && i < renameTo.size(); ++i)
    {
        if (m_settings.m_selectedChannel == renameFrom.at(i)) {
            m_settings.m_selectedChannel = renameTo.at(i);
        }
    }

    updateChannelList();
    applySettings();
    updateFrequencyText();
}

void FreqDisplayGUI::on_channels_currentIndexChanged(int index)
{
    if ((index >= 0) && (index < m_availableChannels.size())) {
        m_settings.m_selectedChannel = m_availableChannels.at(index).getLongId();
    } else {
        m_settings.m_selectedChannel.clear();
    }

    applySettings();
    updateFrequencyText();
}

void FreqDisplayGUI::pollSelectedChannel()
{
    updateFrequencyText();
}

void FreqDisplayGUI::updateFrequencyText()
{
    if (m_settings.m_selectedChannel.isEmpty())
    {
        ui->frequencyValue->setText(tr("No channel selected"));
        updateFrequencyFont();
        return;
    }

    const int channelListIndex = m_availableChannels.indexOfLongId(m_settings.m_selectedChannel);

    if (channelListIndex < 0)
    {
        ui->frequencyValue->setText(tr("Selected channel unavailable"));
        updateFrequencyFont();
        return;
    }

    const auto& selectedChannel = m_availableChannels.at(channelListIndex);
    double centerFrequencyHz = 0.0;
    int offsetHz = 0;

    if (!ChannelWebAPIUtils::getCenterFrequency(selectedChannel.m_superIndex, centerFrequencyHz))
    {
        ui->frequencyValue->setText(tr("Frequency unavailable"));
        updateFrequencyFont();
        return;
    }
    if (!ChannelWebAPIUtils::getFrequencyOffset(selectedChannel.m_superIndex, selectedChannel.m_index, offsetHz))
    {
        ui->frequencyValue->setText(tr("Offset unavailable"));
        updateFrequencyFont();
        return;
    }

    const qint64 centerFrequency = qRound64(centerFrequencyHz);
    const qint64 channelOffset = static_cast<qint64>(offsetHz);
    const qint64 absoluteFrequency = centerFrequency + channelOffset;
    ui->frequencyValue->setText(tr("%1 Hz").arg(QLocale().toString(absoluteFrequency)));
    updateFrequencyFont();
}

void FreqDisplayGUI::updateFrequencyFont()
{
    const int minDimension = qMin(ui->frequencyValue->width(), ui->frequencyValue->height());
    if (minDimension <= 0) {
        return;
    }
    const int pointSize = qMax(minimumFrequencyFontPointSize, static_cast<int>(minDimension * frequencyFontScale));

    QFont font = ui->frequencyValue->font();
    font.setPointSize(pointSize);
    ui->frequencyValue->setFont(font);
}

void FreqDisplayGUI::resizeEvent(QResizeEvent *event)
{
    FeatureGUI::resizeEvent(event);
    updateFrequencyFont();
}
