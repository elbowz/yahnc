#pragma once

#include <Adafruit_BME280.h>
#include <EnvironmentCalculations.h>

#include "BaseNode.hpp"
#include "SensorBase.hpp"
#include "PressureSensor.hpp"

class PressureSensor;

/**
 * TODO:
 * * create a MultiSensorNode that aggregate SensorBase through a std::vector<SensorBase*> sensors.
 * * add destructor (eg. delete pointers) or use std::unique_ptr<>/std::shared_ptr<>
 * * move all advertise in setup() ?
 */

class BME280Node : public BaseNode {
private:
    uint8_t mI2cAddress;
    bool mSensorFound = false;
    HomieSetting<double> *mTempOffsetSetting;

    Adafruit_BME280 bme280;
    Adafruit_BME280::sensor_sampling mTempSampling;
    Adafruit_BME280::sensor_sampling mPressSampling;
    Adafruit_BME280::sensor_sampling mHumSampling;
    Adafruit_BME280::sensor_filter mFilter;

    // Different ways to add/use sensors to the node
    friend class PressureSensor;

    class Temperature : public SensorBase<float> {
    private:
        BME280Node &mNode;
        Adafruit_Sensor *mAdafruitSensor;

    protected:
        float readMeasurement() override;

        void sendMeasurement(float value) const override;

    public:
        explicit Temperature(BME280Node &node, const char *name = cTemperatureTopic, uint32_t readInterval = 4 * 1000UL, uint8_t sendOnChangeRate = 0, float sendOnChangeAbs = 0.1);

        void setup();
    };

protected:
    void setup() override;

    void onReadyToOperate() override;

    void loop() override;

    void sendHumidity(float value) const;

    float readHumidity();

public:
    Temperature mTempSensor;
    PressureSensor *mPressureSensor;
    SensorBase<float> mHumiditySensor;

    explicit BME280Node(const char *id,
                        const char *name,
                        uint8_t i2cAddress = 0x76,
                        Adafruit_BME280::sensor_sampling tempSampling = Adafruit_BME280::SAMPLING_X1,
                        Adafruit_BME280::sensor_sampling pressSampling = Adafruit_BME280::SAMPLING_X1,
                        Adafruit_BME280::sensor_sampling humSampling = Adafruit_BME280::SAMPLING_X1,
                        Adafruit_BME280::sensor_filter filter = Adafruit_BME280::FILTER_OFF);
};