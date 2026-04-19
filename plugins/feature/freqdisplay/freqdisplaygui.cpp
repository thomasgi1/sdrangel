#include <QFont>
#include <QLocale>
#include <QMdiArea>
#include <QResizeEvent>
#include <QTimer>

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
    m_doApplySettings(true),
    m_savedMdi(nullptr)
{
    (void) pluginAPI;
    (void) featureUISet;


    m_feature = feature;
    setAttribute(Qt::WA_DeleteOnClose, true);
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
    connect(ui->speech, &ButtonSwitch::toggled, this, &FreqDisplayGUI::on_speech_toggled);
    connect(ui->fontFamily, &QFontComboBox::currentFontChanged, this, &FreqDisplayGUI::on_fontFamily_currentFontChanged);
    connect(ui->transparentBackground, &ButtonSwitch::toggled, this, &FreqDisplayGUI::on_transparentBackground_toggled);
    connect(&m_pollTimer, &QTimer::timeout, this, &FreqDisplayGUI::pollSelectedChannel);
    m_pollTimer.start(pollIntervalMs);

    displaySettings();
    updateFrequencyText();
    m_resizer.enableChildMouseTracking();
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

    ui->speech->blockSignals(true);
    ui->speech->setChecked(m_settings.m_speechEnabled);
    ui->speech->blockSignals(false);

    ui->transparentBackground->blockSignals(true);
    ui->transparentBackground->setChecked(m_settings.m_transparentBackground);
    ui->transparentBackground->blockSignals(false);

    applyTransparency();
    applySpeech();
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
    settingsKeys.append("speechEnabled");
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
    auto setLabelText = [this](const QString& text) {
        ui->frequencyValue->setText(text);
#ifdef QT_TEXTTOSPEECH_FOUND
        if (m_settings.m_speechEnabled && m_speech && (text != m_previousDisplayText))
        {
            if (m_speech->state() == QTextToSpeech::Speaking)
            {
                // Engine is busy — save the latest text so the stateChanged
                // slot can say it once the current utterance finishes.
                m_pendingSpeechText = text;
            }
            else
            {
                m_pendingSpeechText.clear();
                m_speech->say(text);
            }
        }
#endif
        m_previousDisplayText = text;
    };

    if (m_settings.m_selectedChannel.isEmpty())
    {
        setLabelText(tr("No channel selected"));
        updateFrequencyFont();
        return;
    }

    const int channelListIndex = m_availableChannels.indexOfLongId(m_settings.m_selectedChannel);

    if (channelListIndex < 0)
    {
        setLabelText(tr("Selected channel unavailable"));
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
            setLabelText(tr("Frequency unavailable"));
            updateFrequencyFont();
            return;
        }
        if (!ChannelWebAPIUtils::getFrequencyOffset(selectedChannel.m_superIndex, selectedChannel.m_index, offsetHz))
        {
            setLabelText(tr("Offset unavailable"));
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
            setLabelText(tr("Power unavailable"));
            updateFrequencyFont();
            return;
        }
        powerText = QString("%1 dBFS").arg(power, 0, 'f', 1);
    }

    // --- Compose display text ---
    if (mode == FreqDisplaySettings::Frequency) {
        setLabelText(freqText);
    } else if (mode == FreqDisplaySettings::Power) {
        setLabelText(powerText);
    } else {
        setLabelText(freqText + "\n" + powerText);
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
    int pointSize = qMax(minimumFrequencyFontPointSize, qMin(maxFromWidth, maxFromHeight));
    font.setPointSize(pointSize);

    // Verify the text actually fits at the calculated point size.  The linear
    // interpolation from fontProbePointSize can be slightly inaccurate due to
    // font hinting or non-linear glyph metrics at the target size; if the text
    // would overflow and trigger word-wrap, reduce by one point.  This is a
    // single-step correction that is sufficient because the interpolation error
    // is at most a fraction of one point size.
    if (pointSize > minimumFrequencyFontPointSize)
    {
        const QFontMetrics verifyFm(font);
        for (const QString& line : lines)
        {
            if (verifyFm.horizontalAdvance(line) > availableWidth)
            {
                font.setPointSize(--pointSize);
                break;
            }
        }
    }

    ui->frequencyValue->setFont(font);
}

void FreqDisplayGUI::applyTransparency()
{
    setStyleSheet(m_normalStyleSheet);

    RollupContents *rollupContents = getRollupContents();

    if (m_settings.m_transparentBackground)
    {
        // WA_TranslucentBackground must be set before the native window handle is
        // created or recreated.  Setting it here (before the deferred lambda's
        // setWindowFlags call, which forces native-window recreation) is the
        // correct place.  For the startup case where the widget has not yet been
        // shown, setting it here is also safe.
        setAttribute(Qt::WA_TranslucentBackground, true);

        // Detach from the MDI area so that WA_TranslucentBackground operates at the
        // OS compositor level.  Inside a QMdiArea the attribute only affects Qt's
        // internal backing store, which only passes through for QOpenGLWidget children
        // (GPU compositing path) but not for ordinary software-rendered siblings or
        // external application windows.
        if (!m_savedMdi && mdiArea())
        {
            m_savedMdi = mdiArea();
            // Save position in MDI viewport coordinates and convert to screen
            // coordinates before the window is reparented.
            m_mdiGeometry = geometry();
            const QPoint globalPos = m_savedMdi->viewport()->mapToGlobal(m_mdiGeometry.topLeft());
            // Defer the reparenting to the next event loop iteration so that any
            // in-progress paint or layout event completes first.
            QTimer::singleShot(0, this, [this, globalPos]() {
                if (!m_savedMdi) { return; }
                m_savedMdi->removeSubWindow(this);
                // FeatureGUI is always frameless (Qt::FramelessWindowHint is set in
                // FeatureGUI's constructor) and removeSubWindow() already sets the
                // Qt::Window flag.  Only the always-on-top hint needs to be added so
                // the overlay floats above the main window.
                setWindowFlag(Qt::WindowStaysOnTopHint, true);
                move(globalPos);
                resize(m_mdiGeometry.size());
                show();
            });
        }

        rollupContents->setTransparentBackground(true);
        ui->settingsContainer->setAutoFillBackground(true);
        // Hide the controls bar so only frequencyValue fills the window.
        // RollupContents::arrangeRollups() is triggered automatically by the
        // Hide event and will reposition/resize horizontalWidget to use all
        // available space.
        ui->settingsContainer->hide();
    }
    else
    {
        if (m_savedMdi)
        {
            QMdiArea* savedMdi = m_savedMdi;
            const QRect savedMdiGeometry = m_mdiGeometry;
            m_savedMdi = nullptr;
            // Convert current screen position back to MDI viewport coordinates so
            // the window reappears where the user last placed it.
            const QPoint currentGlobalPos = mapToGlobal(QPoint(0, 0));
            const QPoint mdiPos = savedMdi->viewport()->mapFromGlobal(currentGlobalPos);
            // Defer re-embedding to the next event loop iteration.
            QTimer::singleShot(0, this, [this, savedMdi, mdiPos, savedMdiGeometry]() {
                // Hide first to prevent a flash of the overlay appearing as an
                // opaque window before it is re-embedded in the MDI area.
                hide();
                // Clear translucency before native-window recreation so the
                // window is rebuilt as opaque.
                setAttribute(Qt::WA_TranslucentBackground, false);
                // Remove the WindowStaysOnTopHint that was added when entering
                // transparent mode; without this the re-embedded QMdiSubWindow
                // retains the hint and the title bar / border do not reappear.
                setWindowFlag(Qt::WindowStaysOnTopHint, false);
                savedMdi->addSubWindow(this);
                show();
                move(mdiPos);
                resize(savedMdiGeometry.size());
            });
        }

        rollupContents->setTransparentBackground(false);
        ui->settingsContainer->setAutoFillBackground(false);
        ui->settingsContainer->setStyleSheet(QString());
        ui->horizontalWidget->setStyleSheet(QString());
        ui->frequencyValue->setStyleSheet(QString());
        // Restore the controls bar that was hidden while in transparent mode.
        ui->settingsContainer->show();
    }
}

void FreqDisplayGUI::applySpeech()
{
#ifdef QT_TEXTTOSPEECH_FOUND
    if (m_settings.m_speechEnabled && !m_speech)
    {
        m_speech = new QTextToSpeech(this);
        connect(m_speech, &QTextToSpeech::stateChanged, this, &FreqDisplayGUI::speechStateChanged);
    }
#endif
}

void FreqDisplayGUI::on_displayMode_currentIndexChanged(int index)
{
    m_settings.m_displayMode = static_cast<FreqDisplaySettings::DisplayMode>(index);
    applySettings();
    updateFrequencyText();
}

void FreqDisplayGUI::on_speech_toggled(bool checked)
{
    m_settings.m_speechEnabled = checked;
    applySpeech();
    applySettings();
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

#ifdef QT_TEXTTOSPEECH_FOUND
void FreqDisplayGUI::speechStateChanged(QTextToSpeech::State state)
{
    if (state == QTextToSpeech::Ready && !m_pendingSpeechText.isEmpty())
    {
        const QString text = m_pendingSpeechText;
        m_pendingSpeechText.clear();
        m_speech->say(text);
    }
}
#endif
