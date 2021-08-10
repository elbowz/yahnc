#include "AdcNode.hpp"

extern int __get_adc_mode();

AdcNode::AdcNode(const char *id, const char *name,
                 uint32_t readInterval, float sendOnChangeAbs,
                 const SensorBase<float>::OnChangeFunc &onChangeFunc)
        : BaseNode(id, name, "photoresistorNode"),
          SensorBase(id, readInterval, 0, sendOnChangeAbs, nullptr, nullptr, onChangeFunc) {

    // ADC pin used to measure VCC (e.g. on battery)
    // true if previously called macro ADC_MODE(ADC_VCC)
    mReadVcc = __get_adc_mode() == ADC_VCC;
}

void AdcNode::setup() {

    advertise(getId())
    .setDatatype("float")
    .setFormat("0:1.00")
    .setUnit(cUnitVolt);
}

void AdcNode::loop() {
    SensorBase::loop();
}

float AdcNode::readMeasurement() {
    return static_cast<float>(mReadVcc ? ESP.getVcc() : analogRead(A0)) / 1024.0f;
}

void AdcNode::sendMeasurement(float value) const {

    if(Homie.isConnected()) {
        setProperty(getId()).send(String(value));
    }
}