#include <Asprintf_P/Asprintf_P.h>

#include "PressureSensor.hpp"

PressureSensor::PressureSensor(BME280Node &node, const char *name, uint32_t readInterval, uint8_t sendOnChangeRate, float sendOnChangeAbs)
        : SensorBase<float>(name, readInterval, sendOnChangeRate, sendOnChangeAbs),
          mNode(node) {

    sensor_t sensor;
    mAdafruitSensor = mNode.bme280.getPressureSensor();
    mAdafruitSensor->getSensor(&sensor);

    char *format;
    asprintf(&format, F("%2.2f:%2.2f"), sensor.min_value, sensor.max_value);

    mNode.advertise(getName())
            .setDatatype("float")
            .setFormat(format)
            .setUnit(cUnitHpa);
}

void PressureSensor::setup() {
    mAdafruitSensor->printSensorDetails();
}

float PressureSensor::readMeasurement() {
    //sensors_event_t temperature;
    //mAdafruitSensor->getEvent(&temperature);

    mNode.bme280.takeForcedMeasurement();
    return mNode.bme280.readPressure();
}

void PressureSensor::sendMeasurement(float value) const {

    float pressureHpa = value / 100.0f;

    Homie.getLogger() << mNode.cIndent << getName() << F(": ") << pressureHpa << " " << cUnitHpa << " due reason: " << (int)mUpdateReason << endl;

    mNode.setProperty(cPressureTopic).send(String(pressureHpa));
}
