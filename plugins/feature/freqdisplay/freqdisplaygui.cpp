#include <QFont>
#include <QLocale>
#include <QResizeEvent>

#include "channel/channelwebapiutils.h"

#include "feature/featureuiset.h"

#include "ui_freqdisplaygui.h"
#include "freqdisplay.h"
#include "freqdisplaygui.h"

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
    m_availableChannelOrFeatureHandler(QStringList(), "RT"),
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
    m_pollTimer.start(1000);

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

    int i = 0;
    for (const auto& availableChannel : m_availableChannels)
    {
        const QString longId = availableChannel.getLongId();
        ui->channels->addItem(longId);

        if (longId == m_settings.m_selectedChannel) {
            selectedIndex = i;
        }

        ++i;
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

    if (!ChannelWebAPIUtils::getCenterFrequency(selectedChannel.m_superIndex, centerFrequencyHz)
        || !ChannelWebAPIUtils::getFrequencyOffset(selectedChannel.m_superIndex, selectedChannel.m_index, offsetHz))
    {
        ui->frequencyValue->setText(tr("Frequency unavailable"));
        updateFrequencyFont();
        return;
    }

    const qint64 absoluteFrequency = qRound64(centerFrequencyHz) + static_cast<qint64>(offsetHz);
    ui->frequencyValue->setText(tr("%1 Hz").arg(QLocale().toString(absoluteFrequency)));
    updateFrequencyFont();
}

void FreqDisplayGUI::updateFrequencyFont()
{
    const int minDimension = qMin(ui->frequencyValue->width(), ui->frequencyValue->height());
    const int pointSize = qMax(10, static_cast<int>(minDimension * 0.22));

    QFont font = ui->frequencyValue->font();
    font.setPointSize(pointSize);
    ui->frequencyValue->setFont(font);
}

void FreqDisplayGUI::resizeEvent(QResizeEvent *event)
{
    FeatureGUI::resizeEvent(event);
    updateFrequencyFont();
}
