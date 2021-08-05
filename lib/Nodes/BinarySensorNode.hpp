#pragma once

#include <Bounce2.h>
#include <BaseNode.hpp>
#include <SensorBase.hpp>

/**
 * TODO:
 * * rename to BinaryInNode
 */

using B2Button = Bounce2::Button;

class BinarySensorNode : public BaseNode, public SensorInterface<bool>, public B2Button {
protected:
    void loop() override;

    bool readMeasurement() override;

    void sendMeasurement(bool state) const override;

public:
    explicit BinarySensorNode(const char *id, const char *name,
                              uint8_t pin, uint8_t pinMode = INPUT_PULLUP,
                              uint16_t debounceInterval = 10, uint8_t stateForPressed = LOW,
                              const SensorInterface<bool>::OnChangeFunc &onChangeFunc = [](bool value) { return true; });
};

