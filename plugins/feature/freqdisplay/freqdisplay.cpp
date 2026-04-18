#include <QDebug>

#include "freqdisplay.h"

const char* const FreqDisplay::m_featureIdURI = "sdrangel.feature.freqdisplay";
const char* const FreqDisplay::m_featureId = "FreqDisplay";

FreqDisplay::FreqDisplay(WebAPIAdapterInterface *webAPIAdapterInterface) :
    Feature(m_featureIdURI, webAPIAdapterInterface)
{
    qDebug("FreqDisplay::FreqDisplay: webAPIAdapterInterface: %p", webAPIAdapterInterface);
    setObjectName(m_featureId);
    m_state = StIdle;
}

bool FreqDisplay::handleMessage(const Message& cmd)
{
    (void) cmd;
    return false;
}

QByteArray FreqDisplay::serialize() const
{
    return m_settings.serialize();
}

bool FreqDisplay::deserialize(const QByteArray& data)
{
    if (!m_settings.deserialize(data)) {
        m_settings.resetToDefaults();
        return false;
    }

    return true;
}

void FreqDisplay::applySettings(const FreqDisplaySettings& settings, const QStringList& settingsKeys, bool force)
{
    if (force) {
        m_settings = settings;
    } else {
        m_settings.applySettings(settingsKeys, settings);
    }
}
