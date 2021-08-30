#include "AdcNode.hpp"

extern int __get_adc_mode();

/**
 * TODO:
 * * manage status topic: isnan(ESP.getVcc()) => setProperty(cStatusTopic).send("error")
 *   note: check what happen when no wire is connected to ADC pin
 */

AdcNode::AdcNode(const char *id,
                 const char *name,
                 uint32_t readInterval,
                 float sendOnChangeAbs,
                 const SensorBase<float>::ReadMeasurementFunc &readMeasurementFunc,
                 const SensorBase<float>::SendMeasurementFunc &sendMeasurementFunc,
                 const SensorBase<float>::OnChangeFunc &onChangeFunc)
        : BaseNode(id, name, "adc"),
          SensorBase(id, readInterval, 0, sendOnChangeAbs, readMeasurementFunc, sendMeasurementFunc, onChangeFunc) {

    // ADC pin used to measure VCC (e.g. on battery)
    // true if previously called macro ADC_MODE(ADC_VCC)
    mReadVcc = __get_adc_mode() == ADC_VCC;
}

void AdcNode::setup() {

    advertise(SensorBase::getName())
            .setDatatype("float")
            .setFormat("0:1.00")
            .setUnit(cUnitVolt);
}

void AdcNode::loop() {
    SensorBase::loop();
}

float AdcNode::readMeasurement() {

    if (mReadMeasurementFunc) {
        return mReadMeasurementFunc();
    }

    return static_cast<float>(mReadVcc ? ESP.getVcc() : analogRead(A0)) / 1024.0f;
}

void AdcNode::sendMeasurement(float value) const {

    if (mSendMeasurementFunc) {
        return mSendMeasurementFunc(value);
    }

    if (Homie.isConnected()) {
        setProperty(SensorBase::getName()).send(String(value));
    }
}
