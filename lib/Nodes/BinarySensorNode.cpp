#include "BinarySensorNode.hpp"

BinarySensorNode::BinarySensorNode(const char *id, const char *name,
                                   uint8_t pin, uint8_t pinMode, uint16_t debounceInterval, uint8_t stateForTrue,
                                   const SensorInterface<bool>::OnChangeFunc &onChangeFunc)
        : BaseNode(id, name, "binary"),
          SensorInterface<bool>(name, onChangeFunc),
          B2Button() {

    // init Bounce2
    B2Button::attach(pin, pinMode);
    B2Button::interval(debounceInterval);
    B2Button::setPressedState(stateForTrue);

    advertise("state").setDatatype("boolean");
}

void BinarySensorNode::loop() {
    B2Button::update();

    if (B2Button::changed()) {

        bool state = readMeasurement();
        sendMeasurement(state);

        //printCaption();
        Homie.getLogger() << cIndent << BaseNode::getName() << F(" is ") << (state ? F("true") : F("false")) << endl;
    }
}

bool BinarySensorNode::readMeasurement() {
    return (B2Button::read() == B2Button::getPressedState());
}

void BinarySensorNode::sendMeasurement(bool state) const {

    if(onChange(state) && Homie.isConnected()) {
        setProperty("state").send(state ? F("true") : F("false"));
    }
}
