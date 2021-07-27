#pragma once

#include <BaseNode.hpp>
#include <SensorBase.hpp>

class AdcNode : public BaseNode, public SensorBase<float> {
    typedef std::function<float(float)> BeforeSendFunc;
private:
    bool mReadVcc;
    BeforeSendFunc mBeforeSendFunc;
protected:
    void setup() override;

    void onReadyToOperate() override;

    void loop() override;

    float readMeasurement() override;

    void sendMeasurement(float value) const override;

public:
    AdcNode(const char *id, const char *name,
            uint32_t readInterval = 1 * 1000UL, float sendOnChangeAbs = 0.06f,
            const AdcNode::BeforeSendFunc &beforeSendFunc = [](float value) { return value; },
            const SensorInterface<float>::OnChangeFunc &onChangeFunc = [](bool value) { return true; });

};