#ifndef INCLUDE_FEATURE_FREQDISPLAY_H_
#define INCLUDE_FEATURE_FREQDISPLAY_H_

#include <QStringList>

#include "feature/feature.h"

#include "freqdisplaysettings.h"

class WebAPIAdapterInterface;

class FreqDisplay : public Feature
{
    Q_OBJECT
public:
    FreqDisplay(WebAPIAdapterInterface *webAPIAdapterInterface);
    ~FreqDisplay() override = default;

    void destroy() override { delete this; }
    bool handleMessage(const Message& cmd) override;

    void getIdentifier(QString& id) const override { id = objectName(); }
    QString getIdentifier() const override { return objectName(); }
    void getTitle(QString& title) const override { title = m_settings.m_title; }

    QByteArray serialize() const override;
    bool deserialize(const QByteArray& data) override;

    const FreqDisplaySettings& getSettings() const { return m_settings; }
    void applySettings(const FreqDisplaySettings& settings, const QStringList& settingsKeys, bool force = false);

    static const char* const m_featureIdURI;
    static const char* const m_featureId;

private:
    FreqDisplaySettings m_settings;
};

#endif // INCLUDE_FEATURE_FREQDISPLAY_H_
