#include "ButtonNode.hpp"

ButtonNode::ButtonNode(const char *id, const char *name,
                       uint8_t pin, uint8_t pinMode,
                       uint16_t debounceInterval, uint8_t pinValueForPressed,
                       uint8_t maxMultiPressCount, uint16_t maxMultiPressInterval,
                       const ButtonNode::OnButtonChangeFunc &onChangeFunc)
        : BaseNode(id, name, "button"),
          BinarySensorNode(id, name, pin, pinMode, debounceInterval, pinValueForPressed),
          mMaxMultiPressCount(maxMultiPressCount),
          mMaxMultiPressInterval(maxMultiPressInterval),
          mOnChangeFunc(onChangeFunc) {

    //BinarySensorNode::setOnChangeFunc([=](bool state) { return this->onChange(state, 0, 0); });
    mEvent.button = static_cast<Bounce2::Button *>(this);

    advertise("press-duration").setDatatype("integer").setUnit(cUnitTimeMs);
    advertise("press-count").setDatatype("integer").setUnit(cUnitCount);
}

void ButtonNode::loop() {
    BinarySensorNode::loop();

    /* Button hold */
    if (B2Button::isPressed()) {
        // True at each iteration with: button press and debounced
        mEvent.type = ButtonEventType::HOLD;
        mEvent.duration.current = B2Button::currentDuration();
        mEvent.duration.previous = B2Button::previousDuration();

        onChange(mEvent);
    }

    /* Button press count */
    if (B2Button::pressed()) {
        // True once with: button press and debounced
        if (!mMultiPressStartTime) {
            // init gap/interval to
            mMultiPressStartTime = millis();
        }

        // note: not used mEvent.pressCount due the use of union (ie. value overwritten by current/duration)
        mPressCount++;

    } else if (B2Button::released()) {

        if (mPressCount >= mMaxMultiPressCount) {
            // reached the allowed max press
            sendPressCount(false);

            mMultiPressStartTime = mPressCount = 0;
        }
    } else if (mMultiPressStartTime && millis() - mMultiPressStartTime > mMaxMultiPressInterval) {
        // reached the allowed max interval
        if (mPressCount >= 1) {
            sendPressCount(true);
        }

        mMultiPressStartTime = mPressCount = 0;
    }
}

void ButtonNode::sendPressCount(bool maxInterval) {

    // init mEvent
    mEvent.type = maxInterval ? ButtonEventType::MULTI_PRESS_INTERVAL : ButtonEventType::MULTI_PRESS_COUNT;
    mEvent.pressCount = mPressCount;

    if (onChange(mEvent) && Homie.isConnected()) {
        setProperty("press-count").send(String(mEvent.pressCount));
    }

    /*Homie.getLogger() << BaseNode::getName() << F(" mEvent.pressCount ") << mEvent.pressCount << F(" in ")
                      << millis() - mMultiPressStartTime << (maxInterval ? F(" (MAX INTERVAL)") : F(" (MAX PRESS)"))
                      << endl;*/
}

bool ButtonNode::onChange(bool state) {
    // note: "state" publishing is in charge of BinarySensorNode if this func return true

    // init mEvent ("current" is unuseful in these events: PRESS/RELEASE)
    mEvent.type = state ? ButtonEventType::PRESS : ButtonEventType::RELEASE;
    mEvent.duration.current = 0;
    mEvent.duration.previous = B2Button::previousDuration();

    bool allowSend = onChange(mEvent);

    if (Homie.isConnected() && !state && allowSend) {
        // on ButtonEventType::RELEASE publish "press-duration"
        setProperty("press-duration").send(String(mEvent.duration.previous));
        //Homie.getLogger() << BaseNode::getName() << "press-duration: " << mEvent.duration.previous << endl;
    }

    return allowSend;
}

bool ButtonNode::onChange(const ButtonEvent &event) {
    return mOnChangeFunc(mEvent);
}
