#pragma once

#include "BaseNode.hpp"
#include "ActuatorBase.hpp"

/**
 * TODO:
 * * pass node id to mSetHwStateFunc, mGetHwStateFunc ?
 * * allow to retrieve state from external modification (light impulsive switch with rotary relè):
 *      * state from another pin
 *      * loop on getHwState
 *      note: maybe a new class that inherit form SwitchNode and BinarySensorNode?
 * * rename to BinaryOutNode
 * * rename reverseSignal in stateForTrue/On ?
 * * new feature: restore state after lost power: On, Off and Remember last mode (maybe inside ActuatorBase?)
 *   (hint: EEPROM.read(); EEPROM.write(SHUTTERS_EEPROM_POSITION, state); EEPROM.commit();)
 */

class SwitchNode : virtual public BaseNode, public ActuatorBase<bool> {
protected:
    int8_t mPin;
    uint8_t mOnValue;
    Ticker mTicker;

    bool handleInput(const HomieRange &range, const String &property, const String &value) override;

    void onReadyToOperate() override;

    void setup() override;

public:
    explicit SwitchNode(const char *id,
                        const char *name,
                        int8_t pin = cDisabledPin,
                        uint8_t pinValueForTrue = HIGH,
                        const OnSetFunc &onSetFunc = [](bool value) { return true; },
                        const GetStateFunc &getStateFunc = nullptr,
                        const SetHwStateFunc &setHwStateFunc = nullptr,
                        const SendStateFunc &sendStateFunc = nullptr);

    /**
     * Get pin/hw state.
     * Overriding if you need to change it or set by constructor/getStateFunc
     * @return state, not the pin value (ie. LOW/HIGH)
     */
    bool getState() override;

    /**
     * Set pin/hw state.
     * Overriding if you need to change it or set by constructor/setHwStateFunc
     * @param value switch state, not the pin value (ie. LOW/HIGH)
     */
    void setHwState(bool value) override;

    /**
     * Send/setProperty
     * Overriding if you need to change it or set by constructor/setSendStateFunc
     * @param on switch state, not the pin value (ie. LOW/HIGH)
     */
    void sendState(bool value) const override;

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

    void stopTimeout();
};
