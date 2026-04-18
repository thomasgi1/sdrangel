#ifndef INCLUDE_FREQDISPLAY_WEBAPIADAPTER_H
#define INCLUDE_FREQDISPLAY_WEBAPIADAPTER_H

#include "feature/featurewebapiadapter.h"

#include "freqdisplaysettings.h"

class FreqDisplayWebAPIAdapter : public FeatureWebAPIAdapter {
public:
    FreqDisplayWebAPIAdapter() = default;
    ~FreqDisplayWebAPIAdapter() override = default;

    QByteArray serialize() const override { return m_settings.serialize(); }
    bool deserialize(const QByteArray& data) override { return m_settings.deserialize(data); }
    void setSettings(const FreqDisplaySettings& settings) { m_settings = settings; }
    const FreqDisplaySettings& getSettings() const { return m_settings; }

private:
    FreqDisplaySettings m_settings;
};

#endif // INCLUDE_FREQDISPLAY_WEBAPIADAPTER_H
