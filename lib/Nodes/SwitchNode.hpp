#pragma once

#include <BaseNode.hpp>

class SwitchNode : public BaseNode {
public:
    typedef std::function<bool()> GetStateFunc;
    typedef std::function<void(bool)> SetStateFunc;
    typedef std::function<void(bool)> OnChangeFunc;

private:
    static const long cDefaultMaxTimeout = 600;

    int8_t mPin;
    uint8_t mOnValue;
    uint8_t mOffValue;
    Ticker mTicker;
    HomieSetting<long> *mMaxTimeoutSetting;
    OnChangeFunc mOnChangeFunc;
    SetStateFunc mSetStateFunc;
    GetStateFunc mGetStateFunc;

protected:
    bool handleInput(const HomieRange &range, const String &property, const String &value) override;

    void onReadyToOperate() override;

    void setup() override;

    virtual void onChange(bool value);

    void setStateWrapper(bool on);

    bool getStateWrapper();

public:
    explicit SwitchNode(const char *id,
                        const char *name,
                        int8_t pin = cDisabledPin,
                        bool reverseSignal = false,
                        const SwitchNode::OnChangeFunc &onChangeFunc = [](bool value) {},
                        const SwitchNode::SetStateFunc &setStateFunc = nullptr,
                        const SwitchNode::GetStateFunc &getStateFunc = nullptr);

    SwitchNode &setGetStateFunc(const GetStateFunc &func) {
        mGetStateFunc = func;
        return *this;
    }

    SwitchNode &setSetStateFunc(const SetStateFunc &func) {
        mSetStateFunc = func;
        return *this;
    }

    SwitchNode &setOnChangeFunc(const OnChangeFunc &func) {
        mOnChangeFunc = func;
        return *this;
    }

    /**
     * Get switch state logic. Overriding if you need to change it or set by constructor/setGetStateFunc
     * @return switch state, not the pin value (ie. LOW/HIGH)
     */
    virtual bool getState();

    /**
     * Set switch state logic. Overriding if you need to change it or set by constructor/setSetStateFunc
     * @param on switch state, note the pin value (ie. LOW/HIGH)
     */
    virtual void setState(bool on);

    void setTimeout(uint32_t timeout = 0);

};
