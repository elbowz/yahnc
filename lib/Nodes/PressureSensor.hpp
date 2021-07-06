#pragma once

#include <Adafruit_BME280.h>

#include "BME280Node.hpp"
#include "SensorBase.hpp"
#include "constants.hpp"

class BME280Node;

class PressureSensor : public SensorBase<float> {
private:
    BME280Node &mNode;
    Adafruit_Sensor *mAdafruitSensor;

protected:
    float readMeasurement() override;

    void sendMeasurement(float value) const override;

public:
    explicit PressureSensor(BME280Node &node, const char *name = cPressureTopic, uint32_t readInterval = 60 * 1000UL, uint8_t sendOnChangeRate = 0, float sendOnChangeAbs = 10);

    void setup();
};