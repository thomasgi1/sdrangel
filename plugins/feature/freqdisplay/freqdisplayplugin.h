#ifndef INCLUDE_FEATURE_FREQDISPLAYPLUGIN_H
#define INCLUDE_FEATURE_FREQDISPLAYPLUGIN_H

#include <QObject>

#include "plugin/plugininterface.h"

class FeatureGUI;
class WebAPIAdapterInterface;

class FreqDisplayPlugin : public QObject, PluginInterface {
    Q_OBJECT
    Q_INTERFACES(PluginInterface)
    Q_PLUGIN_METADATA(IID "sdrangel.feature.freqdisplay")

public:
    explicit FreqDisplayPlugin(QObject* parent = nullptr);

    const PluginDescriptor& getPluginDescriptor() const;
    void initPlugin(PluginAPI* pluginAPI);

    FeatureGUI* createFeatureGUI(FeatureUISet *featureUISet, Feature *feature) const override;
    Feature* createFeature(WebAPIAdapterInterface *webAPIAdapterInterface) const override;
    FeatureWebAPIAdapter* createFeatureWebAPIAdapter() const override;

private:
    static const PluginDescriptor m_pluginDescriptor;

    PluginAPI* m_pluginAPI;
};

#endif // INCLUDE_FEATURE_FREQDISPLAYPLUGIN_H
