#include "AdcNode.hpp"

extern int __get_adc_mode();

AdcNode::AdcNode(const char *id, const char *name,
                 uint32_t readInterval, float sendOnChangeAbs,
                 const AdcNode::BeforeSendFunc &beforeSendFunc,
                 const SensorInterface<float>::OnChangeFunc &onChangeFunc)
        : BaseNode(id, name, "adcNode"),
          SensorBase(name, readInterval, 0, sendOnChangeAbs, nullptr, nullptr, onChangeFunc),
          mBeforeSendFunc(beforeSendFunc) {

    // true if previously called macro ADC_MODE(ADC_VCC)
    mReadVcc = __get_adc_mode() == ADC_VCC;

    advertise(cVoltageTopic)
            .setDatatype("float")
            .setFormat("??:??")
            .setUnit(cUnitVolt);
}

void AdcNode::loop() {
    // Call loop ONLY if connected to MQTT
    if(Homie.isConnected()) SensorBase::loop();
}

void AdcNode::setup() {
    HomieNode::setup();
}

void AdcNode::onReadyToOperate() {
    HomieNode::onReadyToOperate();
}

float AdcNode::readMeasurement() {
    //Homie.getLogger() << "READ VALUE " << static_cast<float>(mReadVcc ? ESP.getVcc() : analogRead(A0)) / 1024.0f << endl;
    return static_cast<float>(mReadVcc ? ESP.getVcc() : analogRead(A0)) / 1024.0f;
}

void AdcNode::sendMeasurement(float value) const {
    if (onChange(value)) {
        value = mBeforeSendFunc(value);
        setProperty(cVoltageTopic).send(String(value));
    }
}
