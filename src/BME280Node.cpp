#include <Asprintf_P/Asprintf_P.h>

#include "BME280Node.hpp"

BME280Node::BME280Node(const char *id,
                       const char *name,
                       uint8_t i2cAddress,
                       const Adafruit_BME280::sensor_sampling tempSampling,
                       const Adafruit_BME280::sensor_sampling pressSampling,
                       const Adafruit_BME280::sensor_sampling humSampling,
                       const Adafruit_BME280::sensor_filter filter)
        : BaseNode(id, name, "BME280"),
          mI2cAddress(i2cAddress),
          mTempSampling(tempSampling),
          mPressSampling(pressSampling),
          mHumSampling(humSampling),
          mFilter(filter),
          mTempSensor(*this),
          mHumiditySensor(cHumidityTopic, 8 * 1000UL, 0, 1) {

    mHumiditySensor
            .setReadMeasurementFunc(std::bind(&BME280Node::readHumidity, this))
                    // pre C++ 11 way
                    //.setSendMeasurementFunc(std::bind(&BME280Node::sendHumidity, this, std::placeholders::_1));
                    // post C++ 11, lambda function way
            .setSendMeasurementFunc([=](float value) { this->sendHumidity(value); });

    mPressureSensor = new PressureSensor(*this);

    // TODO: call free() or use std::unique_ptr<>/std::shared_ptr<Base>
    // note: this object will be destroyed only on reset of ESP8266 (ie. not necessary to free up memory)
    //free((void *) mSettingTempOffset->getName()); free((void *) mSettingTempOffset->getDescription());
    //std::shared_ptr<char> test(new char[strlen_P(cSettingTempOffsetName)+2], std::default_delete<char[]>());

    char *settingName;
    char *settingDescription;
    asprintf(&settingName, F("%s.temperatureOffset"), id);
    asprintf(&settingDescription, F("The temperature offset in degrees [-10.0, 10.0] Default = %f"), cDefaultTempOffset);

    mSettingTempOffset = new HomieSetting<double>(settingName, settingDescription);

    mSettingTempOffset
            ->setDefaultValue(cDefaultTempOffset)
            .setValidator([](float candidate) {
                return (candidate >= -10.0f) && (candidate <= 10.0f);
            });

    advertise(cStatusTopic)
            .setDatatype("enum")
            .setFormat("error, ok");

    advertise(cHumidityTopic)
            .setDatatype("percent")
            .setFormat("0:100")
            .setUnit(cUnitPercent);

    advertise(cAbsHumidityTopic)
            .setDatatype("float")
            .setFormat("0:30")
            .setUnit(cUnitMgm3);
}

/**
 * Called ONCE, when the device is in normal mode.
 * Connection was only initiated but not established.
 */
void BME280Node::setup() {

    //printCaption();

    Homie.getLogger() << "setup()" << endl;

    if (bme280.begin(mI2cAddress)) {

        mSensorFound = true;
        Homie.getLogger() << cIndent << F("found. Reading interval: ") << 300 << " s" << endl;

        bme280.setSampling(Adafruit_BME280::MODE_FORCED, mTempSampling, mPressSampling, mHumSampling, mFilter);
        bme280.setTemperatureCompensation(static_cast<float>(mSettingTempOffset->get()));

        auto mAdafruitSensor = bme280.getHumiditySensor();
        mAdafruitSensor->printSensorDetails();

        mTempSensor.setup();
        mPressureSensor->setup();
    } else {

        mSensorFound = false;
        Homie.getLogger() << cIndent << F("not found. Check wiring!") << endl;
    }

    // TODO: maybe we can clear the setting name/description there
    // free((void *) mSettingTempOffset->getName()); free((void *) mSettingTempOffset->getDescription());
}

/*
 * Called each time MQTT connection is done or restored
 */
void BME280Node::onReadyToOperate() {
    Homie.getLogger() << "onReadyToOperate()" << endl;

    setProperty(cStatusTopic).send(mSensorFound ? F("ok") : F("error"));
}

/*
 * Continually called when the device is in normal mode (after configuration and connection is done)
 */
void BME280Node::loop() {

    // Check if Connected to avoid the warning msg "setNodeProperty(): impossible now"
    // To figure out when isConnected is init and loop is called, see:
    // * https://github.com/homieiot/homie-esp8266/blob/9cd83972f27b394eab8a5e3e2baef20eea1b5408/src/SendingPromise.cpp#L43
    // * https://github.com/homieiot/homie-esp8266/blob/9cd83972f27b394eab8a5e3e2baef20eea1b5408/src/Homie/Boot/BootNormal.cpp#L125-L146
    if (Homie.isConnected() && mSensorFound) {
        mTempSensor.loop();
        mHumiditySensor.loop();
        mPressureSensor->loop();
    }
}

void BME280Node::sendHumidity(float value) const {

    Homie.getLogger() << cIndent << mHumiditySensor.getName() << F(":  ") << value << " " << cUnitPercent << " due reason: " << this->mHumiditySensor.getIntUpdateReason() << endl;
    float absHumidity = EnvironmentCalculations::AbsoluteHumidity(mTempSensor, value, EnvironmentCalculations::TempUnit_Celsius);

    setProperty(cHumidityTopic).send(String(value));
    setProperty(cAbsHumidityTopic).send(String(absHumidity));
}

float BME280Node::readHumidity() {

    bme280.takeForcedMeasurement(); // has no effect in normal mode
    return bme280.readHumidity();
}

BME280Node::TemperatureSensor::TemperatureSensor(BME280Node &node, const char *name, uint32_t readInterval, uint8_t sendOnChangeRate, float sendOnChangeAbs)
        : SensorBase<float>(name, readInterval, sendOnChangeRate, sendOnChangeAbs),
          mNode(node) {

    sensor_t sensor;
    mAdafruitSensor = mNode.bme280.getTemperatureSensor();
    mAdafruitSensor->getSensor(&sensor);

    char *format;
    asprintf(&format, F("%2.2f:%2.2f"), sensor.min_value, sensor.max_value);

    mNode.advertise(getName())
            .setDatatype("float")
            .setFormat(format)
            .setUnit(cUnitDegrees);

    mNode.advertise(cHeatIndexTopic)
            .setDatatype("float")
            .setFormat(format)
            .setUnit(cUnitDegrees);
}

void BME280Node::TemperatureSensor::setup() {
    mAdafruitSensor->printSensorDetails();
}

float BME280Node::TemperatureSensor::readMeasurement() {
    //sensors_event_t temperature;
    //mAdafruitSensor->getEvent(&temperature);

    mNode.bme280.takeForcedMeasurement();
    return mNode.bme280.readTemperature();
}

void BME280Node::TemperatureSensor::sendMeasurement(float value) const {

    Homie.getLogger() << mNode.cIndent << getName() << F(": ") << value << " " << cUnitDegrees << " due reason: " << (int) mUpdateReason << endl;
    float heatIndex = EnvironmentCalculations::HeatIndex(value, mNode.mHumiditySensor, EnvironmentCalculations::TempUnit_Celsius);

    mNode.setProperty(cTemperatureTopic).send(String(value));
    mNode.setProperty(cHeatIndexTopic).send(String(heatIndex));
}

