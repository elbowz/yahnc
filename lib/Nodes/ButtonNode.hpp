#pragma once

#include <BinarySensorNode.hpp>

/**
 * TODO:
 * * put enum and struct inside ?
 */

enum class ButtonEventType : uint8_t {
    PRESS = 1,
    RELEASE,
    HOLD,
    MULTI_PRESS_COUNT,
    MULTI_PRESS_INTERVAL,
};

struct ButtonEvent {
    ButtonEventType type;
    Bounce2::Button *button;
    union {
        uint8_t pressCount;
        struct {
            uint32_t current;
            uint32_t previous;
        } duration;
    };
};

class ButtonNode : public BinarySensorNode {
public:
    typedef std::function<bool(const ButtonEvent &)> OnButtonChangeFunc;
protected:
    ButtonEvent mEvent;
    uint8_t mPressCount = 0;
    uint32_t mMultiPressStartTime = 0;
    uint8_t mMaxMultiPressCount;
    uint16_t mMaxMultiPressInterval;
    OnButtonChangeFunc mOnChangeFunc;

    void loop() override;

    void sendPressCount(bool maxInterval = false);

    bool onChange(bool value) override;

    /**
     * Called in 4 different case (ie. ButtonEventType) with relevant attributes:
     *  * PRESS: Button pushed - duration.previous
     *  * RELEASE: Button released - duration.previous
     *  * HOLD: Continuously called during button press - duration.current, duration.previous
     *  * MULTI_PRESS_COUNT: Multi press maxMultiPressCount reached - pressCount
     *  * MULTI_PRESS_INTERVAL: Multi press maxMultiPressInterval reached - pressCount
     * type and button attributes are always relevant, ignore the remains.
     * This function can be set or override through inheritance way.
     * note: the onChange exist also in SensorInterface, but this is enriched with more params
     * @return true to allow the sendMeasurement invocation
     */
    virtual bool onChange(const ButtonEvent &event);

public:
    ButtonNode(const char *id, const char *name,
               uint8_t pin, uint8_t pinMode = INPUT_PULLUP,
               uint16_t debounceInterval = 10, uint8_t pinValueForPressed = LOW,
               uint8_t maxMultiPressCount = 3, uint16_t maxMultiPressInterval = 1000,
               const ButtonNode::OnButtonChangeFunc &onChangeFunc = [](const ButtonEvent &event) { return true; });

    ButtonNode &setOnChangeFunc(const OnButtonChangeFunc &func) {
        mOnChangeFunc = func;
        return *this;
    }
};
