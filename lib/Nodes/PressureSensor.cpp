#include "PressureSensor.hpp"

PressureSensor::PressureSensor(BME280Node &node, const char *name, uint32_t readInterval, uint8_t sendOnChangeRate, float sendOnChangeAbs)
        : SensorBase<float>(name, readInterval, sendOnChangeRate, sendOnChangeAbs),
          mNode(node) {

    sensor_t sensor;
    mAdafruitSensor = mNode.bme280.getPressureSensor();
    mAdafruitSensor->getSensor(&sensor);

    static char format[15];
    snprintf ( format, 15, "%2.2f:%2.2f", sensor.min_value, sensor.max_value);
    //static const char* format =  String(sensor.min_value + String(':') + sensor.max_value).c_str();

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
