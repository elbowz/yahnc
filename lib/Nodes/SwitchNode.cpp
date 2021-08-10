#include <Asprintf_P.h>

#include "SwitchNode.hpp"

SwitchNode::SwitchNode(const char *id, const char *name, int8_t pin, bool reverseSignal,
                       const OnSetFunc &onSetFunc,
                       const GetStateFunc &getStateFunc,
                       const SetHwStateFunc &setHwStateFunc,
                       const SendStateFunc &sendStateFunc)
        : BaseNode(id, name, "switch"),
          ActuatorBase<bool>(id, onSetFunc, getStateFunc, setHwStateFunc, sendStateFunc),
          mPin(pin) {

    //asprintf(&_caption, "• %s relay pin[%d]:", name, pin);

    mOnValue = reverseSignal ? LOW : HIGH;
}

void SwitchNode::setup() {
    if (mPin > cDisabledPin) {
        pinMode(mPin, OUTPUT);
    }

    // note: inputHandler is set and handleInput return true
    // to still allow the external use of "SwitchNode::getProperty("on")->settable(lightOnHandler)" by user
    advertise(getId())
    .setDatatype("boolean")
    .settable([](const HomieRange &range, const String &value) { return true; });

    char *format;
    asprintf(&format, F("0:%lld"), ULONG_MAX);

    advertise("timeout")
    .setDatatype("integer")
    .setFormat(format)
    .settable([](const HomieRange &range, const String &value) { return true; });
}

void SwitchNode::onReadyToOperate() {

    // TODO: retrieve value from mqtt or set the start default value
    setState(false);
    setTimeout(0);
};

bool SwitchNode::handleInput(const HomieRange &range, const String &property, const String &value) {

    if (property == getId()) {

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

bool SwitchNode::getState() {

    if (mGetStateFunc) {
        return mGetStateFunc();
    } else if (mPin > cDisabledPin) {
        return (digitalRead(mPin) == mOnValue);
    } else {
        HomieInternals::Helpers::abort(F("✖ pin and getStateFunc() are both not set!"));
    }

    return false;
}

void SwitchNode::setHwState(bool on) const {

    if (mSetHwStateFunc) {
        mSetHwStateFunc(on);
    } else if (mPin > cDisabledPin) {
        digitalWrite(mPin, on ? mOnValue : !mOnValue);
    } else {
        HomieInternals::Helpers::abort(F("✖ pin and setHwStateFunc() are both not set!"));
    }
}

void SwitchNode::sendState(bool value) const {

    if (Homie.isConnected()) {
        setProperty(getId()).send(value ? "true" : "false");
    }
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
        if (getState() != endState) setState(endState);
    }
}

void SwitchNode::stopTimeout() {
    mTicker.detach();
}


