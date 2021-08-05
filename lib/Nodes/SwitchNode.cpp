#include <Asprintf_P.h>

#include "SwitchNode.hpp"

/**
 * TODO:
 * pass node id to mSetStateFunc, mGetStateFunc
 * allow to check state from external modification (light impulsive switch): state from another pin, loop on getState and setState normal/only mqtt topic
 * implement onChange directly in RetentionVar
 */

SwitchNode::SwitchNode(const char *id, const char *name, int8_t pin, bool reverseSignal,
                       const SwitchNode::OnChangeFunc &onChangeFunc,
                       const SwitchNode::SetHwStateFunc &setHwStateFunc,
                       const SwitchNode::GetHwStateFunc &getHwStateFunc)
        : BaseNode(id, name, "switch"),
          mPin(pin),
          mOnChangeFunc(onChangeFunc),
          mSetHwStateFunc(setHwStateFunc),
          mGetHwStateFunc(getHwStateFunc) {

    //asprintf(&_caption, "• %s relay pin[%d]:", name, pin);

    mOnValue = reverseSignal ? LOW : HIGH;
    mOffValue = !mOnValue;

    // note: inputHandler is set and handleInput return true
    // to still allow the external use of "SwitchNode::getProperty("on")->settable(lightOnHandler)" by user
    advertise("on")
            .setDatatype("boolean")
            .settable([](const HomieRange &range, const String &value) { return true; });

    char *format;
    asprintf(&format, F("0:%lld"), ULONG_MAX);

    advertise("timeout")
            .setDatatype("integer")
            .setFormat(format)
            .settable([](const HomieRange &range, const String &value) { return true; });
}

void SwitchNode::setup() {
    if (mPin > cDisabledPin) {
        pinMode(mPin, OUTPUT);
    }
}

void SwitchNode::onReadyToOperate() {

    // TODO: retrieve value from mqtt or set the start default value
    setState(false);
    setTimeout(0);
};

bool SwitchNode::handleInput(const HomieRange &range, const String &property, const String &value) {

    if (property == "on") {

        if (value != "true" && value != "false" && value != "toggle") return false;

        bool newStatus = (value == "toggle" ? !getState() : value == "true");
        setState(newStatus);

        return false;

    } else if (property == "timeout") {

        uint32_t timeout = value.toInt();
        if (value != String(timeout) && timeout <= 0) return false;

        setTimeoutWithInit(timeout, true, false);

        return false;
    }

    return false;
}

void SwitchNode::setState(bool on) {

    if (onChange(on)) {
        setHwState(on);
    }

    Homie.getLogger() << cIndent << F("is ") << (on ? F("on") : F("off")) << endl;

    if (Homie.isConnected()) {
        setProperty("on").send(getState() ? "true" : "false");
    }
}

void SwitchNode::setHwState(bool on) {

    if (mSetHwStateFunc) {
        mSetHwStateFunc(on);
    } else if (mPin > cDisabledPin) {
        digitalWrite(mPin, on ? mOnValue : mOffValue);
    } else {
        HomieInternals::Helpers::abort(F("✖ pin and setStateFunc() are both not set!"));
    }
}

// Exist only to allow the inheritance of getHwState() by the user
inline bool SwitchNode::getState() {
    return getHwState();
}

bool SwitchNode::getHwState() {

    if (mGetHwStateFunc) {
        return mGetHwStateFunc();
    } else if (mPin > cDisabledPin) {
        return (digitalRead(mPin) == mOnValue);
    } else {
        HomieInternals::Helpers::abort(F("✖ pin and getStateFunc() are both not set!"));
    }
    return false;
}

void SwitchNode::setTimeoutWithInit(uint32_t seconds, bool startState, bool endState) {

    setState(startState);
    setTimeout(seconds, endState);
}

void SwitchNode::setTimeout(uint32_t seconds, bool endState) {

    String stringTimeout = String(seconds);
    Homie.getLogger() << cIndent << F("seconds: ") << stringTimeout << endl;

    if (Homie.isConnected()) {
        setProperty("timeout").send(String(seconds));
    }

    if (seconds > 0) {
        mTicker.once_scheduled(1, std::bind(&SwitchNode::setTimeout, this, seconds - 1, endState));
    } else {
        if (getHwState() != endState) setState(endState);
    }
}

bool SwitchNode::onChange(bool value) {
    return mOnChangeFunc(value);
}

void SwitchNode::stopTimeout() {
    mTicker.detach();
}


