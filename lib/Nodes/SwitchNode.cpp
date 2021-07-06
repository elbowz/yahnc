#include "SwitchNode.hpp"

//TODO:
// * pass node id to mSetStateFunc, mGetStateFunc
// * allow to check state from external modification (light impulsive switch): state from another pin, loop on getState and setState normal/only mqtt topic
// * implement onChange directly in RetentionVar

SwitchNode::SwitchNode(const char *id, const char *name, int8_t pin, bool reverseSignal,
                       const SwitchNode::OnChangeFunc &onChangeFunc,
                       const SwitchNode::SetStateFunc &setStateFunc,
                       const SwitchNode::GetStateFunc &getStateFunc)
        : BaseNode(id, name, "switch"),
          mPin(pin),
          mOnChangeFunc(onChangeFunc),
          mSetStateFunc(setStateFunc),
          mGetStateFunc(getStateFunc) {

    //asprintf(&_caption, "• %s relay pin[%d]:", name, pin);

    mOnValue = reverseSignal ? LOW : HIGH;
    mOffValue = !mOnValue;

    auto maxTimeoutName = String(id) + ".maxTimeout";
    mMaxTimeoutSetting = new HomieSetting<long>(maxTimeoutName.c_str(),
                                                "The maximum timeout for the relay in seconds [0 .. Max(long)] Default = 600 (10 minutes)");

    mMaxTimeoutSetting->setDefaultValue(cDefaultMaxTimeout).setValidator([](long candidate) {
        return (candidate >= 0);
    });

    // note: inputHandler is set and handleInput return false
    // to still allow the externally use of "SwitchNode::getProperty("on")->settable(lightOnHandler)" by user
    advertise("on")
            .setDatatype("boolean")
            .settable([](const HomieRange& range, const String& value) { return true; });

    static char format[15];
    snprintf(format, 15, "0:%ld", mMaxTimeoutSetting->get());
    //static const char *format = String(String("0:") + mMaxTimeoutSetting->get()).c_str();

    advertise("timeout")
            .setDatatype("integer")
            .setFormat(format)
            .settable([](const HomieRange& range, const String& value) { return true; });
}

void SwitchNode::setup() {
    if (mPin > cDisabledPin) {
        pinMode(mPin, OUTPUT);
    }
}

void SwitchNode::onReadyToOperate() {
    // TODO: retrieve value from mqtt or set the start default value
    setStateWrapper(false);
    setTimeout(0);
};

bool SwitchNode::handleInput(const HomieRange &range, const String &property, const String &value) {

    if (property == "on") {

        if (value != "true" && value != "false" && value != "toggle") return false;

        bool newStatus = (value == "toggle" ? !getStateWrapper() : value == "true");
        setStateWrapper(newStatus);

        return false;

    } else if (property == "timeout") {

        uint32_t timeout = value.toInt();
        if (value != String(timeout) && timeout <= 0) return false;

        setStateWrapper(true);
        setTimeout(timeout);

        return false;
    }

    return false;
}

void SwitchNode::setStateWrapper(bool on) {
    setState(on);
    onChange(on);
    setProperty("on").send(getStateWrapper() ? "true" : "false");

    Homie.getLogger() << cIndent << F("is ") << (on ? F("on") : F("off")) << endl;
}

void SwitchNode::setState(bool on) {
    if (mSetStateFunc) {
        mSetStateFunc(on);
    } else if (mPin > cDisabledPin) {
        digitalWrite(mPin, on ? mOnValue : mOffValue);
    } else {
        HomieInternals::Helpers::abort(F("✖ pin and setStateFunc() are both not set!"));
    }
}

// Exist only to allow the inheritance of getState() by the user
inline bool SwitchNode::getStateWrapper() {
    return getState();
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

void SwitchNode::setTimeout(uint32_t timeout) {
    bool currentState = getStateWrapper();
    String stringTimeout = String(timeout);

    setProperty("timeout").send(String(timeout));
    Homie.getLogger() << cIndent << F("timeout: ") << stringTimeout << endl;

    if (timeout) {
        mTicker.once_scheduled(1, std::bind(&SwitchNode::setTimeout, this, timeout-1));
    } else {
        if (currentState) setStateWrapper(false);
    }
}

void SwitchNode::onChange(bool value) {
    mOnChangeFunc(value);
}


