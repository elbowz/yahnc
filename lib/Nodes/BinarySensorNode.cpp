#include "BinarySensorNode.hpp"

BinarySensorNode::BinarySensorNode(const char *id, const char *name,
                                   uint8_t pin, uint8_t pinMode, uint16_t debounceInterval, uint8_t pinStateForTrue,
                                   const SensorInterface<bool>::OnChangeFunc &onChangeFunc)
        : BaseNode(id, name, "binary"),
          SensorInterface<bool>(id, onChangeFunc),
          B2Button() {

    // init Bounce2
    B2Button::attach(pin, pinMode);
    B2Button::interval(debounceInterval);
    B2Button::setPressedState(pinStateForTrue);

    advertise(getId()).setDatatype("boolean");
}

void BinarySensorNode::loop() {
    B2Button::update();

    if (B2Button::changed()) {

        bool state = readMeasurement();

        if (onChange(state)) {
            sendMeasurement(state);
        }
    }
}

bool BinarySensorNode::readMeasurement() {
    return B2Button::isPressed();
}

void BinarySensorNode::sendMeasurement(bool state) const {

    if (Homie.isConnected()) {
        setProperty(getId()).send(state ? F("true") : F("false"));
    }
}
