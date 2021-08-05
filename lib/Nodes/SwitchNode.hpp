#pragma once

#include <BaseNode.hpp>

/**
 * TODO:
 * * pass node id to mSetHwStateFunc, mGetHwStateFunc ?
 * * allow to retrieve state from external modification (light impulsive switch with rotary relè):
 *      * state from another pin
 *      * loop on getHwState
 *      note: maybe a new class that inherit form SwitchNode and BinarySensorNode?
 * * inherit from SensorInterface (getHwState => readMeasuremente, onChange, part of setState => sendMeasurement)
 * * rename to BinaryOutNode
 */

class SwitchNode : public BaseNode {
public:
    typedef std::function<bool()> GetHwStateFunc;
    typedef std::function<void(bool)> SetHwStateFunc;
    typedef std::function<bool(bool)> OnChangeFunc;

private:
    int8_t mPin;
    uint8_t mOnValue;
    uint8_t mOffValue;
    Ticker mTicker;
    OnChangeFunc mOnChangeFunc;
    SetHwStateFunc mSetHwStateFunc;
    GetHwStateFunc mGetHwStateFunc;

protected:
    bool handleInput(const HomieRange &range, const String &property, const String &value) override;

    void onReadyToOperate() override;

    void setup() override;

    virtual bool onChange(bool value);

public:
    explicit SwitchNode(const char *id,
                        const char *name,
                        int8_t pin = cDisabledPin,
                        bool reverseSignal = false,
                        const SwitchNode::OnChangeFunc &onChangeFunc = [](bool value) { return true; },
                        const SwitchNode::SetHwStateFunc &setHwStateFunc = nullptr,
                        const SwitchNode::GetHwStateFunc &getHwStateFunc = nullptr);

    SwitchNode &setGetHwStateFunc(const GetHwStateFunc &func) {
        mGetHwStateFunc = func;
        return *this;
    }

    SwitchNode &setSetHwStateFunc(const SetHwStateFunc &func) {
        mSetHwStateFunc = func;
        return *this;
    }

    SwitchNode &setOnChangeFunc(const OnChangeFunc &func) {
        mOnChangeFunc = func;
        return *this;
    }

    void stopTimeout();

    /**
     * Get pin/hw state. Overriding if you need to change it or set by constructor/setGetHwStateFunc
     * @return state, not the pin value (ie. LOW/HIGH)
     */
    virtual bool getHwState();

    /**
     * Set pin/hw state. Overriding if you need to change it or set by constructor/setSetHwStateFunc
     * @param on switch state, not the pin value (ie. LOW/HIGH)
     */
    virtual void setHwState(bool on);

    /**
    * Set pin/hw state and update mqtt.
    * @param on state, not the pin value (ie. LOW/HIGH)
    */
    void setState(bool on);

    /**
     * alias of getHwState()
     * @return state, not the pin value (ie. LOW/HIGH)
     */
    bool getState();

    /**
     * Set endState after a predefined seconds
     * @param seconds delay in s
     * @param endState state to set at the end of time
     * @param killOther true, take place of current timer (if exist)
     */
    void setTimeout(uint32_t seconds = 0, bool endState = false);

    /**
     * Same of setTimeout with a pre call to setState
     */
    void setTimeoutWithInit(uint32_t seconds = 0, bool startState = true, bool endState = false);

};
