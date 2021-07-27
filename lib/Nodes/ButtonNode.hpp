#pragma once

#include <BinarySensorNode.hpp>

class ButtonNode : public BinarySensorNode {
public:
    typedef std::function<bool(bool, uint32_t, uint8_t)> OnButtonChangeFunc;
private:
    void _sendPressCount(bool maxInterval = false);

    bool onChange(bool value) const override;

protected:
    uint8_t mPressCount = 0;
    uint32_t mMultiPressStartTime = 0;
    uint8_t mMaxMultiPressCount;
    uint16_t mMaxMultiPressInterval;
    OnButtonChangeFunc mOnChangeFunc;

    void loop() override;

    /**
     * Called in 3 different case:
     *  * on buttonNode state change. duration = pressCount = 0
     *  * on buttonNode release. state = false, pressCount = 0
     *  * on MaxMultiPress Count or Interval reached. pressCount = 0
     * This function can be set or override through inheritance way.
     * note: the onChange exist also in SensorInterface, but this is enriched with more params
     * @return true to allow the sendMeasurement invocation
     */
    virtual bool onChange(bool value, uint32_t duration = 0, uint8_t pressCount = 0) const;

public:
    ButtonNode(const char *id, const char *name,
               uint8_t pin, uint8_t pinMode = INPUT_PULLUP,
               uint16_t debounceInterval = 10, uint8_t stateForTrue = LOW,
               uint8_t maxMultiPressCount = 3, uint16_t maxMultiPressInterval = 1000,
               const ButtonNode::OnButtonChangeFunc &onChangeFunc = [](bool state, uint32_t duration, uint8_t pressCount) { return true; });
};
