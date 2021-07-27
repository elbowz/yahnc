// TemperatureHUmidityMotionBuzzerLight-Pressure
#define AP_NAME "BDIot"
#define FW_NAME "THUMBL-P"
#define FW_VERSION "0.0.6b"

#ifndef SERIAL_SPEED
#define SERIAL_SPEED 115200
#endif

#include <map>
#include <Homie.h>

#define BUZZER_PIN 13
//TODO: try tu put in PROGMEM
std::map<const String, const char *> rtttlMelodies = {
        {"two-short", ":d=4,o=5,b=100:16e6,16e6"},
        {"one-long",  ":d=1,o=5,b=100:e6"},
        {"siren",     ":d=8,o=5,b=100:d,e,d,e,d,e,d,e"},
        {"scale-up",  ":d=32,o=5,b=100:c,c#,d#,e,f#,g#,a#,b"},
        {"tetris",    ":d=4,o=5,b=160:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a"}
};

#include <NonBlockingRtttl.h>

// Nodes
#include <BME280Node.hpp>
#include <SwitchNode.hpp>
#include <BinarySensorNode.hpp>
#include <ButtonNode.hpp>
#include <AdcNode.hpp>

// For read ESP8266 VCC voltage
//ADC_MODE(ADC_VCC);
const int BME280_I2C_ADDRESS = 0x76; // Default I2C address for BME280. can be changed to 0x76 by changing a solder bridge

// more events on: https://homieiot.github.io/homie-esp8266/docs/3.0.1/advanced-usage/events/
void onHomieEvent(const HomieEvent &event) {
    switch (event.type) {
        case HomieEventType::MQTT_READY:
            rtttl::begin(BUZZER_PIN, rtttlMelodies.find("scale-up")->second);
            break;
        default:
            break;
    }
}

float adcVoltageToLux(float value) {
    // Voltage divider (1st in Nodemcu, 2nd with R1(photoresistor) and R2
    // Vin = value*3.3 (for the different range of Nodemcu: 3.3v instead of 1v)
    // R1(photoresistor) = (3.3*R2/Vin)-R2 = (3.3*1k/3.3*value)-1k
    float kOhm = (1 / value) - 1;
    // https://en.wikipedia.org/wiki/Log%E2%80%93log_plot
    // see the GL5528 datasheet (fig.2)
    // we have a log-log scale,
    // y = 100 - x (y = mx + b) on a linear scale ...should be y = 100 - 0.98x
    // log(Lux) = log(100) - log(R1) => Lux = 100/R1
    float Lux = 100 / kOhm;
    Homie.getLogger() << "kOhm : " << kOhm << " Lux: " << Lux << endl;
    return Lux;
}

bool buzzerHandler(const HomieRange &range, const String &property, const String &value);

// Create one node of each kind
BME280Node bme280Node("bme280", "Outdoor", BME280_I2C_ADDRESS);
SwitchNode ledNode("light", "Led light", 15);
BinarySensorNode pirNode("motion", "Motion detector", 12, INPUT, 5, HIGH);
ButtonNode buttonNode("button", "Test button", 14, INPUT_PULLUP, 20, LOW, 3, 1000, [](bool state, uint32_t duration, uint8_t pressCount) {
    Homie.getLogger() << "buttonNode" << endl;
    if (pressCount == 2) rtttl::begin(BUZZER_PIN, rtttlMelodies.find("tetris")->second, 2, 2000);
    else if (duration >= 2000) rtttl::stop();

    return true;
});
AdcNode adcNode("luminance", "Luminance test", 1000, 0.01f, adcVoltageToLux);

// TODO add a constructor overload to Homie Project:
// HomieNode(const char* id, const char* name, const char* type, const HomieInternals::NodeInputHandler& nodeInputHandler = [](const HomieRange& range, const String& property, const String& value) { return false; });
HomieNode buzzerNode("buzzer", "Rtttl buzzer", "Sound", false, 0, 0, buzzerHandler);


bool buzzerHandler(const HomieRange &range, const String &property, const String &value) {

    if (property == "play") {

        if (value.charAt(0) != '{') {    // Melody name (between predefined ones)

            auto melody = rtttlMelodies.find(value.c_str());
            if (melody == rtttlMelodies.end()) return false;

            rtttl::begin(BUZZER_PIN, melody->second);
        } else {                               // Custom melody json
            // TODO: add repeat
        }

        return true;

    } else if (property == "stop") {

        if (value != "true") return false;

        rtttl::stop();

        return true;
    }

    return false;
}

bool broadcastHandler(const String& topic, const String& value) {
    Serial << "Received broadcast " << topic << ": " << value << endl;
    return true;
}

bool globalInputHandler(const HomieNode &node, const HomieRange &range, const String &property, const String &value) {
    Homie.getLogger() << "Received on node " << node.getId() << ": " << property << " = " << value << endl;
    return false;
}

bool onChangeFunc(bool value) {
    Homie.getLogger() << "onChangeFunc " << value << endl;

    return false;
}

bool lightOnHandler(const HomieRange &range, const String &value) {
    Homie.getLogger() << "lightOnHandler " << value << endl;

    return true;
}

bool motionHandler(bool state) {
    Homie.getLogger() << "motion: " << state << endl;
    return true;
}

void homieLoopHandler() {

    // Buzzer MQTT topic "/is-playing" maneging
    static bool lastIsPlaying = false;

    if (rtttl::isPlaying()) {
        if (!lastIsPlaying) {
            buzzerNode.setProperty("is-playing").send(F("true"));
            lastIsPlaying = true;
        }

    } else if (lastIsPlaying) {
        buzzerNode.setProperty("is-playing").send(F("false"));
        lastIsPlaying = false;
    }
}

void setup() {
    Serial.begin(SERIAL_SPEED);

    Homie_setBrand(AP_NAME);                    // Brand in fw, mqtt client name and AP name in configuration mode
    Homie_setFirmware(FW_NAME, FW_VERSION);     // Node name and fw version (for OTA stuff)

    // Initializes I2C for BME280Node sensor
    // Default value. Uncomment and change on need
    //Wire.begin(4, 5);
    pinMode(BUZZER_PIN, OUTPUT);

    // Feedback ledNode for notify configuration mode, connecting to Wi-Fi or connecting to MQTT
    //Homie.disableLedFeedback();
    //Homie.setLedPin(16, HIGH);

    //Homie.disableResetTrigger();
    //Homie.disableLogging();

    //Homie.setSetupFunction();
    Homie.setLoopFunction(homieLoopHandler);
    Homie.onEvent(onHomieEvent);
    Homie.setBroadcastHandler(broadcastHandler);
    Homie.setGlobalInputHandler(globalInputHandler);

    ledNode.setOnChangeFunc(onChangeFunc);
    ledNode.getProperty("on")->settable(lightOnHandler);

    pirNode.setOnChangeFunc(motionHandler);

    // Buzzer MQTT topic/proprieties configuration
    buzzerNode.advertise("play").setDatatype("string").setFormat("json").settable();
    buzzerNode.advertise("stop").setDatatype("boolean").settable();
    buzzerNode.advertise("is-playing").setDatatype("boolean");

    Homie.setup();
}

void loop() {
    Homie.loop();

    if (Homie.isConfigured()) {
        if (rtttl::isPlaying()) rtttl::play();
    }

    // Uncomment for ssl
    // https://github.com/homieiot/homie-esp8266/issues/640
    //delay(100);
    // yield();
}

/* 

* NAN and isnan per i numeri non inizializzati (FUNZIONA SOLO PER FLOAT )

* vector:
  std::vector<HomieNode*> nodes;

  for (HomieNode* iNode : HomieNode::nodes) {
      iNode->onReadyToOperate();
    }

* overraidable / callbackable see:
  "handleInput" in HomieNode.cpp
  and in Callbacks.hpp
  typedef std::function<bool(const HomieRange& range, const String& property, const String& value)> NodeInputHandler;

* HomieSetting<T> for template

* BME280 and unified sensor:
  https://github.com/adafruit/Adafruit_BME280_Library/blob/master/examples/bme280_unified/bme280_unified.ino
  https://github.com/adafruit/Adafruit_Sensor

* platform.io advanced scripting (eg. push config, OTA, etc)
  https://docs.platformio.org/en/latest/projectconf/advanced_scripting.html
*/