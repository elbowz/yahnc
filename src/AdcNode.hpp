#pragma once

#include "BaseNode.hpp"
#include "SensorBase.hpp"

class AdcNode : public BaseNode, public SensorBase<float> {

private:
    bool mReadVcc;

public:
    explicit AdcNode(const char *id, const char *name,
                     uint32_t readInterval = 1 * 1000UL, float sendOnChangeAbs = 0.06f,
                     const SensorBase<float>::ReadMeasurementFunc &readMeasurementFunc = nullptr,
                     const SensorBase<float>::SendMeasurementFunc &sendMeasurementFunc = nullptr,
                     const SensorBase<float>::OnChangeFunc &onChangeFunc = [](bool value) { return true; });

    void setup() override;

    void loop() override;

    float readMeasurement() override;

    void sendMeasurement(float value) const override;
};