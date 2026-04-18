#include <QFont>
#include <QLocale>
#include <QResizeEvent>

#include "channel/channelwebapiutils.h"
#include "gui/buttonswitch.h"

#include "feature/featureuiset.h"

#include "ui_freqdisplaygui.h"
#include "freqdisplay.h"
#include "freqdisplaygui.h"

namespace {
constexpr const char* rxTxChannelKinds = "RT";
constexpr int pollIntervalMs = 1000;
constexpr int minimumFrequencyFontPointSize = 10;
// Reference point size used when probing text metrics in updateFrequencyFont().
// Large enough that integer rounding in QFontMetrics is negligible.
constexpr int fontProbePointSize = 200;
// Stylesheet applied to this window when transparency is enabled.
// The '*' universal selector cascades to every child widget.
constexpr const char* transparentStyleSheet = "* { background: transparent; border: none; }";
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
    // Enable compositor-level transparency so that the window background can be made
    // fully transparent at runtime.  WA_TranslucentBackground must be set before the
    // native window handle is created (i.e., before first show).  The base constructor
    // FeatureGUI(parent) has already run at this point but the widget has not yet been
    // shown, so this is the safe and correct place to set it.
    setAttribute(Qt::WA_TranslucentBackground, true);
    // Capture the stylesheet that FeatureGUI set in its constructor so that
    // applyTransparency() can restore it when transparency is disabled.
    m_normalStyleSheet = styleSheet();

    RollupContents *rollupContents = getRollupContents();
    ui->setupUi(rollupContents);
    ui->frequencyValue->setWordWrap(true);

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
    connect(ui->displayMode, qOverload<int>(&QComboBox::currentIndexChanged), this, &FreqDisplayGUI::on_displayMode_currentIndexChanged);
    connect(ui->fontFamily, &QFontComboBox::currentFontChanged, this, &FreqDisplayGUI::on_fontFamily_currentFontChanged);
    connect(ui->transparentBackground, &ButtonSwitch::toggled, this, &FreqDisplayGUI::on_transparentBackground_toggled);
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

    // Populate font combo box with the saved font (or system default if empty)
    ui->fontFamily->blockSignals(true);
    if (!m_settings.m_fontName.isEmpty()) {
        ui->fontFamily->setCurrentFont(QFont(m_settings.m_fontName));
    }
    ui->fontFamily->blockSignals(false);

    ui->displayMode->blockSignals(true);
    ui->displayMode->setCurrentIndex(static_cast<int>(m_settings.m_displayMode));
    ui->displayMode->blockSignals(false);

    ui->transparentBackground->blockSignals(true);
    ui->transparentBackground->setChecked(m_settings.m_transparentBackground);
    ui->transparentBackground->blockSignals(false);

    applyTransparency();
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
    settingsKeys.append("fontName");
    settingsKeys.append("transparentBackground");
    settingsKeys.append("displayMode");
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
    const FreqDisplaySettings::DisplayMode mode = m_settings.m_displayMode;

    // --- Frequency ---
    QString freqText;
    if (mode == FreqDisplaySettings::Frequency || mode == FreqDisplaySettings::Both)
    {
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

        const qint64 absoluteFrequency = qRound64(centerFrequencyHz) + static_cast<qint64>(offsetHz);
        freqText = tr("%1 Hz").arg(QLocale().toString(absoluteFrequency));
    }

    // --- Power ---
    QString powerText;
    if (mode == FreqDisplaySettings::Power || mode == FreqDisplaySettings::Both)
    {
        double power = 0.0;
        if (!ChannelWebAPIUtils::getChannelReportValue(selectedChannel.m_superIndex, selectedChannel.m_index, "channelPowerDB", power))
        {
            ui->frequencyValue->setText(tr("Power unavailable"));
            updateFrequencyFont();
            return;
        }
        powerText = QString("%1 dBFS").arg(power, 0, 'f', 1);
    }

    // --- Compose display text ---
    if (mode == FreqDisplaySettings::Frequency) {
        ui->frequencyValue->setText(freqText);
    } else if (mode == FreqDisplaySettings::Power) {
        ui->frequencyValue->setText(powerText);
    } else {
        ui->frequencyValue->setText(freqText + "\n" + powerText);
    }

    updateFrequencyFont();
}

void FreqDisplayGUI::updateFrequencyFont()
{
    const int availableWidth = ui->frequencyValue->width();
    const int availableHeight = ui->frequencyValue->height();
    if (availableWidth <= 0 || availableHeight <= 0) {
        return;
    }

    const QString text = ui->frequencyValue->text();
    if (text.isEmpty()) {
        return;
    }

    // Build a font with the user-chosen family (or the widget's current family if none saved)
    QFont font = ui->frequencyValue->font();
    if (!m_settings.m_fontName.isEmpty()) {
        font.setFamily(m_settings.m_fontName);
    }

    // Probe at a large reference size to get accurate text dimensions, then
    // scale linearly to find the largest point size that fits in both directions.
    font.setPointSize(fontProbePointSize);
    const QFontMetrics fm(font);

    // For multi-line text (Both mode) find the widest line; divide available
    // height equally across lines so each line receives the same font size.
    const QStringList lines = text.split('\n');
    const int numLines = lines.size();
    int maxLineWidth = 0;
    for (const QString& line : lines) {
        maxLineWidth = qMax(maxLineWidth, fm.horizontalAdvance(line));
    }
    const int lineHeight = fm.height();

    if (maxLineWidth <= 0 || lineHeight <= 0) {
        return;
    }

    const int heightPerLine = availableHeight / numLines;
    const int maxFromWidth  = fontProbePointSize * availableWidth  / maxLineWidth;
    const int maxFromHeight = fontProbePointSize * heightPerLine   / lineHeight;
    const int pointSize = qMax(minimumFrequencyFontPointSize, qMin(maxFromWidth, maxFromHeight));
    font.setPointSize(pointSize);
    ui->frequencyValue->setFont(font);
}

void FreqDisplayGUI::applyTransparency()
{
    if (m_settings.m_transparentBackground)
    {
        // Override the FeatureGUI stylesheet on the entire window using the '*'
        // universal selector so that every child widget's background is also
        // cleared.  Combined with WA_TranslucentBackground (set in the constructor)
        // this makes the sub-window show whatever is rendered behind it in the
        // MDI area instead of the normal solid background.
        setStyleSheet(transparentStyleSheet);
    }
    else
    {
        // Restore the stylesheet that FeatureGUI set at construction time.
        setStyleSheet(m_normalStyleSheet);
        // Also clear any per-widget overrides from previous transparent runs.
        ui->settingsContainer->setStyleSheet(QString());
        ui->frequencyValue->setStyleSheet(QString());
    }
}

void FreqDisplayGUI::on_displayMode_currentIndexChanged(int index)
{
    m_settings.m_displayMode = static_cast<FreqDisplaySettings::DisplayMode>(index);
    applySettings();
    updateFrequencyText();
}

void FreqDisplayGUI::on_fontFamily_currentFontChanged(const QFont& font)
{
    m_settings.m_fontName = font.family();
    applySettings();
    updateFrequencyFont();
}

void FreqDisplayGUI::on_transparentBackground_toggled(bool checked)
{
    m_settings.m_transparentBackground = checked;
    applyTransparency();
    applySettings();
}

void FreqDisplayGUI::resizeEvent(QResizeEvent *event)
{
    FeatureGUI::resizeEvent(event);
    updateFrequencyFont();
}
