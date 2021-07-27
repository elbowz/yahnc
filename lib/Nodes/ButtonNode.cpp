#include "ButtonNode.hpp"

ButtonNode::ButtonNode(const char *id, const char *name,
                       uint8_t pin, uint8_t pinMode,
                       uint16_t debounceInterval, uint8_t stateForTrue,
                       uint8_t maxMultiPressCount, uint16_t maxMultiPressInterval,
                       const ButtonNode::OnButtonChangeFunc &onChangeFunc)
        : BinarySensorNode(id, name, pin, pinMode, debounceInterval, stateForTrue),
          mMaxMultiPressCount(maxMultiPressCount),
          mMaxMultiPressInterval(maxMultiPressInterval),
          mOnChangeFunc(onChangeFunc) {

    //BinarySensorNode::setOnChangeFunc([=](bool state) { return this->onChange(state, 0, 0); });

    advertise("duration").setDatatype("integer").setUnit(cUnitTimeMs);
    advertise("presscount").setDatatype("integer").setUnit(cUnitCount);
}

void ButtonNode::loop() {
    BinarySensorNode::loop();

    bool btReleased = B2Button::released();

    // Button pressed duration
    if (btReleased) {
        Homie.getLogger() << BaseNode::getName() << "duration: " << B2Button::previousDuration() << endl;
        setProperty("duration").send(String(B2Button::previousDuration()));
        onChange(false, B2Button::previousDuration(), 0);
    }

    // Button count
    if (btReleased) {
        mPressCount++;

        // Catch the buttonNode multi press
        if (!mMultiPressStartTime) { mMultiPressStartTime = millis(); }             // init gap/interval to
        else if (mPressCount >= mMaxMultiPressCount) { _sendPressCount(false); }    // reached the allowed max press

    } else if (mMultiPressStartTime && millis() - mMultiPressStartTime > mMaxMultiPressInterval) {  // reached the allowed max intervall
        _sendPressCount(true);
    }
}

void ButtonNode::_sendPressCount(bool maxInterval) {

    Homie.getLogger() << BaseNode::getName() << F(" mPressCount ") << mPressCount << F(" in ")
                      << millis() - mMultiPressStartTime << (maxInterval ? F(" (MAX INTERVAL)") : F(" (MAX PRESS)"))
                      << endl;

    setProperty("presscount").send(String(mPressCount));

    onChange(B2Button::pressed(), 0, mPressCount);

    mMultiPressStartTime = mPressCount = 0;
}

bool ButtonNode::onChange(bool state) const {
    return ButtonNode::onChange(state, 0, 0);
}

bool ButtonNode::onChange(bool state, uint32_t duration, uint8_t pressCount) const {
    return mOnChangeFunc(state, duration, pressCount);
}
