/**
 * SDRangel
 * This is the web REST/JSON API of SDRangel SDR software. SDRangel is an Open Source Qt5/OpenGL 3.0+ (4.3+ in Windows) GUI and server Software Defined Radio and signal analyzer in software. It supports Airspy, BladeRF, HackRF, LimeSDR, PlutoSDR, RTL-SDR, SDRplay RSP1 and FunCube    ---   Limitations and specifcities:    * In SDRangel GUI the first Rx device set cannot be deleted. Conversely the server starts with no device sets and its number of device sets can be reduced to zero by as many calls as necessary to /sdrangel/deviceset with DELETE method.   * Preset import and export from/to file is a server only feature.   * Device set focus is a GUI only feature.   * The following channels are not implemented (status 501 is returned): ATV and DATV demodulators, Channel Analyzer NG, LoRa demodulator   * The device settings and report structures contains only the sub-structure corresponding to the device type. The DeviceSettings and DeviceReport structures documented here shows all of them but only one will be or should be present at a time   * The channel settings and report structures contains only the sub-structure corresponding to the channel type. The ChannelSettings and ChannelReport structures documented here shows all of them but only one will be or should be present at a time    --- 
 *
 * OpenAPI spec version: 7.0.0
 * Contact: f4exb06@gmail.com
 *
 * NOTE: This class is auto generated by the swagger code generator program.
 * https://github.com/swagger-api/swagger-codegen.git
 * Do not edit the class manually.
 */

/*
 * SWGAudioCATSISOSettings.h
 *
 * AudioCATSISO
 */

#ifndef SWGAudioCATSISOSettings_H_
#define SWGAudioCATSISOSettings_H_

#include <QJsonObject>


#include <QString>

#include "SWGObject.h"
#include "export.h"

namespace SWGSDRangel {

class SWG_API SWGAudioCATSISOSettings: public SWGObject {
public:
    SWGAudioCATSISOSettings();
    SWGAudioCATSISOSettings(QString* json);
    virtual ~SWGAudioCATSISOSettings();
    void init();
    void cleanup();

    virtual QString asJson () override;
    virtual QJsonObject* asJsonObject() override;
    virtual void fromJsonObject(QJsonObject &json) override;
    virtual SWGAudioCATSISOSettings* fromJson(QString &jsonString) override;

    qint64 getRxCenterFrequency();
    void setRxCenterFrequency(qint64 rx_center_frequency);

    qint32 getRxSampleRate();
    void setRxSampleRate(qint32 rx_sample_rate);

    qint64 getTxCenterFrequency();
    void setTxCenterFrequency(qint64 tx_center_frequency);

    qint32 getTxSampleRate();
    void setTxSampleRate(qint32 tx_sample_rate);

    qint32 getTransverterMode();
    void setTransverterMode(qint32 transverter_mode);

    qint64 getTransverterDeltaFrequency();
    void setTransverterDeltaFrequency(qint64 transverter_delta_frequency);

    qint32 getIqOrder();
    void setIqOrder(qint32 iq_order);

    qint32 getTxEnable();
    void setTxEnable(qint32 tx_enable);

    qint32 getPttSpectrumLink();
    void setPttSpectrumLink(qint32 ptt_spectrum_link);

    QString* getRxDeviceName();
    void setRxDeviceName(QString* rx_device_name);

    qint32 getRxIqMapping();
    void setRxIqMapping(qint32 rx_iq_mapping);

    qint32 getLog2Decim();
    void setLog2Decim(qint32 log2_decim);

    qint32 getFcPosRx();
    void setFcPosRx(qint32 fc_pos_rx);

    qint32 getDcBlock();
    void setDcBlock(qint32 dc_block);

    qint32 getIqCorrection();
    void setIqCorrection(qint32 iq_correction);

    float getRxVolume();
    void setRxVolume(float rx_volume);

    QString* getTxDeviceName();
    void setTxDeviceName(QString* tx_device_name);

    qint32 getTxIqMapping();
    void setTxIqMapping(qint32 tx_iq_mapping);

    qint32 getTxVolume();
    void setTxVolume(qint32 tx_volume);

    qint32 getCatSpeedIndex();
    void setCatSpeedIndex(qint32 cat_speed_index);

    qint32 getCatDataBitsIndex();
    void setCatDataBitsIndex(qint32 cat_data_bits_index);

    qint32 getCatStopBitsIndex();
    void setCatStopBitsIndex(qint32 cat_stop_bits_index);

    qint32 getCatHandshakeIndex();
    void setCatHandshakeIndex(qint32 cat_handshake_index);

    qint32 getCatPttMethodIndex();
    void setCatPttMethodIndex(qint32 cat_ptt_method_index);

    qint32 getCatDtrHigh();
    void setCatDtrHigh(qint32 cat_dtr_high);

    qint32 getCatRtsHigh();
    void setCatRtsHigh(qint32 cat_rts_high);

    qint32 getCatPollingMs();
    void setCatPollingMs(qint32 cat_polling_ms);

    qint32 getUseReverseApi();
    void setUseReverseApi(qint32 use_reverse_api);

    QString* getReverseApiAddress();
    void setReverseApiAddress(QString* reverse_api_address);

    qint32 getReverseApiPort();
    void setReverseApiPort(qint32 reverse_api_port);

    qint32 getReverseApiDeviceIndex();
    void setReverseApiDeviceIndex(qint32 reverse_api_device_index);


    virtual bool isSet() override;

private:
    qint64 rx_center_frequency;
    bool m_rx_center_frequency_isSet;

    qint32 rx_sample_rate;
    bool m_rx_sample_rate_isSet;

    qint64 tx_center_frequency;
    bool m_tx_center_frequency_isSet;

    qint32 tx_sample_rate;
    bool m_tx_sample_rate_isSet;

    qint32 transverter_mode;
    bool m_transverter_mode_isSet;

    qint64 transverter_delta_frequency;
    bool m_transverter_delta_frequency_isSet;

    qint32 iq_order;
    bool m_iq_order_isSet;

    qint32 tx_enable;
    bool m_tx_enable_isSet;

    qint32 ptt_spectrum_link;
    bool m_ptt_spectrum_link_isSet;

    QString* rx_device_name;
    bool m_rx_device_name_isSet;

    qint32 rx_iq_mapping;
    bool m_rx_iq_mapping_isSet;

    qint32 log2_decim;
    bool m_log2_decim_isSet;

    qint32 fc_pos_rx;
    bool m_fc_pos_rx_isSet;

    qint32 dc_block;
    bool m_dc_block_isSet;

    qint32 iq_correction;
    bool m_iq_correction_isSet;

    float rx_volume;
    bool m_rx_volume_isSet;

    QString* tx_device_name;
    bool m_tx_device_name_isSet;

    qint32 tx_iq_mapping;
    bool m_tx_iq_mapping_isSet;

    qint32 tx_volume;
    bool m_tx_volume_isSet;

    qint32 cat_speed_index;
    bool m_cat_speed_index_isSet;

    qint32 cat_data_bits_index;
    bool m_cat_data_bits_index_isSet;

    qint32 cat_stop_bits_index;
    bool m_cat_stop_bits_index_isSet;

    qint32 cat_handshake_index;
    bool m_cat_handshake_index_isSet;

    qint32 cat_ptt_method_index;
    bool m_cat_ptt_method_index_isSet;

    qint32 cat_dtr_high;
    bool m_cat_dtr_high_isSet;

    qint32 cat_rts_high;
    bool m_cat_rts_high_isSet;

    qint32 cat_polling_ms;
    bool m_cat_polling_ms_isSet;

    qint32 use_reverse_api;
    bool m_use_reverse_api_isSet;

    QString* reverse_api_address;
    bool m_reverse_api_address_isSet;

    qint32 reverse_api_port;
    bool m_reverse_api_port_isSet;

    qint32 reverse_api_device_index;
    bool m_reverse_api_device_index_isSet;

};

}

#endif /* SWGAudioCATSISOSettings_H_ */
