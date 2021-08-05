// THUMBL-P: TemperatureHUmidityMotionBuzzerLight-Pressure

#define AP_NAME "BDIot"
#define FW_NAME "THUMBL-P"
#define FW_VERSION "0.0.6b"

#include <map>
#include <Homie.h>
#include <NonBlockingRtttl.h>

// Nodes libs
#include <BME280Node.hpp>
#include <SwitchNode.hpp>
#include <BinarySensorNode.hpp>
#include <ButtonNode.hpp>
#include <AdcNode.hpp>

/**
 * TODO:
 *
 */

#ifndef SERIAL_SPEED
#define SERIAL_SPEED 115200
#endif

// TODO: create a PR for Homie prj and add to Statistics:
//  https://github.com/homieiot/homie-esp8266/blob/9cd83972f27b394eab8a5e3e2baef20eea1b5408/src/Homie/Boot/BootNormal.cpp#L175
#ifdef DEBUG
HomieInternals::Timer statsTimer;
#endif

// For read ESP8266 VCC voltage
//ADC_MODE(ADC_VCC);
#define PIN_PIR 12
#define PIN_BUZZER 13
#define PIN_BUTTON 14
#define PIN_LIGHT 15
#define BME280_I2C_ADDRESS 0x76

// Melodies preset
std::map<const String, const char *> rtttlMelodies = {
        {"two-short", ":d=4,o=5,b=100:16e6,16e6"},
        {"one-long",  ":d=1,o=5,b=100:e6"},
        {"siren",     ":d=8,o=5,b=100:d,e,d,e,d,e,d,e"},
        {"scale-up",  ":d=32,o=5,b=100:c,c#,d#,e,f#,g#,a#,b"},
        {"tetris",    ":d=4,o=5,b=160:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a"}
};

#define SETTING_LIGHT_MOTION_TIMEOUT 20
#define SETTING_LIGHT_MOTION_MAX_LUX 10.0f

// Device custom settings
HomieSetting<long> settingLightOnMotionTimeout("lightOnMotionTimeout",
                                               "Light switch-off delay (seconds) after a motion detection [0, " TOSTRING(LONG_MAX) "] Default = " TOSTRING(SETTING_LIGHT_MOTION_TIMEOUT));
HomieSetting<double> settingLightOnMotionMaxLux("lightOnMotionMaxLux",
                                                "Max luminance (lux) to switch-on light on a motion detection [0, 100] Default = " TOSTRING(SETTING_LIGHT_MOTION_MAX_LUX));

// Forward declaration
void homieLoopHandler();

bool buzzerHandler(const HomieRange &range, const String &property, const String &value);

// Extend due the convert ADC analog read to Lux
class GL5528Node : public AdcNode {

    using AdcNode::AdcNode;

    float readMeasurement() override {

        float Vr2 = AdcNode::readMeasurement();

        // Two voltage divider:
        // * 1st with Rp(photoresistor) and R2
        //   Vout = Vr2 = Vin * R2/(Rp+R2) => Rp = (3.3*R2/Vout) - R2 = (3.3*460/Vout) - 460
        // * 2nd in Nodemcu ADC input: range [0,3.3]v, instead of bare esp8266 [0,1]v
        //   Vout = 3.3*Vr2 (scale range: 3.3v instead of 1v)
        // Rp = (3.3*460/3.3*Vr2) - 460 = 460/Vr2 - 460
        float Rp = (460 / Vr2) - 460;

        // Resistance (Ohm) => Luminance (Lux)
        // see the GL5528 datasheet (fig.2)
        // we have a log-log scale (https://en.wikipedia.org/wiki/Log%E2%80%93log_plot)
        // y = 100 - x (y = mx + b) on a linear scale ...should be y = 100 - 0.98x
        // log(lux) = log(100) - log(Rp) => lux = 100/Rp (Rp in KOhm)
        float lux = 100 / (Rp / 1000);

        //Homie.getLogger() << "Luminance - KOhm: " << kOhm << " lux: " << lux << endl;

        return lux;
    };
};


// Nodes instances
SwitchNode ledNode("light", "Led light", PIN_LIGHT);
BME280Node bme280Node("bme280", "BME280", BME280_I2C_ADDRESS);
GL5528Node photoresistorNode("luminance", "Luminance", 1000, 0.80f);
BinarySensorNode pirNode("motion", "Motion detector", PIN_PIR, INPUT, 5, HIGH);
ButtonNode buttonNode("button", "Button", PIN_BUTTON, INPUT_PULLUP, 20, LOW, 3, 1000);
// TODO create a PR for Homie prj and add a constructor overload:
// HomieNode(const char* id, const char* name, const char* type, const HomieInternals::NodeInputHandler& nodeInputHandler = [](const HomieRange& range, const String& property, const String& value) { return false; });
// note: buzzerNode is implemented using Homie "classic method" (ie. no class)
HomieNode buzzerNode("buzzer", "Rtttl buzzer", "Sound", false, 0, 0, buzzerHandler);


/* HANDLERS / CALLBACKS */

// more events on: https://homieiot.github.io/homie-esp8266/docs/3.0.1/advanced-usage/events/
void onHomieEvent(const HomieEvent &event) {
    switch (event.type) {
        case HomieEventType::MQTT_READY:
            rtttl::begin(PIN_BUZZER, rtttlMelodies.find("scale-up")->second);
            break;
        default:
            break;
    }
}

bool broadcastHandler(const String &topic, const String &value) {
    Serial << "Received broadcast " << topic << ": " << value << endl;
    return true;
}

bool globalInputHandler(const HomieNode &node, const HomieRange &range, const String &property, const String &value) {
    Homie.getLogger() << "Received/published an input msg on node " << node.getId() << ": " << property << " = " << value << endl;
    return false;
}

bool buzzerHandler(const HomieRange &range, const String &property, const String &value) {

    if (property == "play") {

        if (value.charAt(0) != '{') {
            // Melody name (between predefined ones)

            auto melody = rtttlMelodies.find(value.c_str());
            if (melody == rtttlMelodies.end()) return false;

            rtttl::begin(PIN_BUZZER, melody->second);
        } else {
            // Custom melody json
            // TODO: add repeat and json decode
        }

        return true;

    } else if (property == "stop") {

        if (value != "true") return false;

        rtttl::stop();

        return true;
    }

    return false;
}

bool buttonHandler(const ButtonEvent &event) {

    // TODO: remove
    if (event.type != ButtonEventType::HOLD) {

        Homie.getLogger() << "ButtonNode - " << "event n°" << static_cast<int>(event.type)
                          << " pressCount: " << event.pressCount
                          << " duration.current: " << event.duration.current
                          << " duration.previous: " << event.duration.previous
                          << endl;
    }

    switch (event.type) {
        case ButtonEventType::HOLD:
            if (event.duration.current >= 2000) rtttl::stop();
            break;
        case ButtonEventType::MULTI_PRESS_COUNT:
        case ButtonEventType::MULTI_PRESS_INTERVAL:
            rtttl::begin(PIN_BUZZER, rtttlMelodies.find("tetris")->second, event.pressCount, 2000);
            break;
        default:
            break;
    }

    return true;
}

bool motionHandler(bool state) {

    auto maxLux = settingLightOnMotionMaxLux.get();
    auto lightTimeout = settingLightOnMotionTimeout.get();

    Homie.getLogger() << "Motion " << state
                      // typecasts overloading (see RetentionVar impl.)
                      << " with " << (float) photoresistorNode << " lux (max " << maxLux << ')'
                      << endl;

    // Switch on light on motion and darkly
    if(state && photoresistorNode.value() < maxLux) {
        ledNode.stopTimeout();
        ledNode.setState(true);
    } else if (!state) {
        // Timeout start only on false because motion stay true if continuously moving ahead it
        // ie. moving time > lightTimeout make light switch off
        ledNode.setTimeout(lightTimeout);
    }

    return true;
}

bool photoresistorHandler(float lux) {
    Homie.getLogger() << "Luminosity: " << lux << " lux" << endl;
    return true;
}


void setup() {
    Serial.begin(SERIAL_SPEED);

    // Initializes I2C for BME280Node sensor
    // use default value. Uncomment and change on need
    //Wire.begin(4, 5);
    pinMode(PIN_BUZZER, OUTPUT);

    // HOMIE SETUP
    Homie_setBrand(AP_NAME);                    // Brand in fw, mqtt client name and AP name in configuration mode
    Homie_setFirmware(FW_NAME, FW_VERSION);     // Node name and fw version (for OTA stuff)

    // Device custom settings: default and validation
    settingLightOnMotionTimeout.setDefaultValue(SETTING_LIGHT_MOTION_TIMEOUT).setValidator([](long candidate) {
        return 0 <= candidate && candidate <= LONG_MAX;
    });

    settingLightOnMotionMaxLux.setDefaultValue(SETTING_LIGHT_MOTION_MAX_LUX).setValidator([](long candidate) {
        return 0 <= candidate && candidate <= 100;
    });


    // Config ledNode for notify
    // * configuration mode (stay on)
    // * normal mode:
    //  * connecting to Wi-Fi (slowly blink)
    //  * connecting to MQTT (faster blink)
    //  * connected (stay off)
    //Homie.disableLedFeedback();
    //Homie.setLedPin(16, HIGH);

    //Homie.disableLogging();
    // pressing for 5 seconds the FLASH (GPIO0) button (default behaviour)
    //Homie.disableResetTrigger();

    //Homie.setSetupFunction();
    Homie.setLoopFunction(homieLoopHandler);
    Homie.onEvent(onHomieEvent);
    Homie.setBroadcastHandler(broadcastHandler);
    Homie.setGlobalInputHandler(globalInputHandler);

    // NODES SETUP
    //ledNode.setOnChangeFunc(onLightChange);
    //ledNode.getProperty("on")->settable(lightOnHandler);

    // call pirNode.loop() even when not connected (but still in normal mode)
    // i.e. some device features must works without connection
    pirNode.setRunLoopDisconnected(true);
    pirNode.setOnChangeFunc(motionHandler);

    photoresistorNode.setRunLoopDisconnected(true);
    photoresistorNode.setOnChangeFunc(photoresistorHandler);

    buttonNode.setRunLoopDisconnected(true);
    buttonNode.setOnChangeFunc(buttonHandler);

    // Buzzer MQTT topic/proprieties configuration
    buzzerNode.advertise("play").setDatatype("string").setFormat("json").settable();
    buzzerNode.advertise("stop").setDatatype("boolean").settable();
    buzzerNode.advertise("is-playing").setDatatype("boolean");

#ifdef DEBUG
    statsTimer.setInterval(5000);
#endif

    Homie.setup();
}

void loop() {
    Homie.loop();

    if (Homie.isConfigured()) {
        if (rtttl::isPlaying()) rtttl::play();
    }

#ifdef DEBUG
    // Check Free Heap memory space
    if(statsTimer.check()) {
        Homie.getLogger() << F("getFreeHeap(): ") << ESP.getFreeHeap() << endl;
        statsTimer.tick();
    }
#endif

    // Uncomment for ssl
    // https://github.com/homieiot/homie-esp8266/issues/640
    //delay(100);
    // or
    //yield();
}

// Homie loop func continuously called in Normal mode and connected
void homieLoopHandler() {

    // Buzzer MQTT topic "/is-playing" managing
    static bool lastIsPlaying = false;

    if (rtttl::isPlaying()) {
        // note: use support var lastIsPlaying because getProperty() implementation is a loop
        if (!lastIsPlaying) {
            buzzerNode.setProperty("is-playing").send(F("true"));
            lastIsPlaying = true;
        }

    } else if (lastIsPlaying) {
        buzzerNode.setProperty("is-playing").send(F("false"));
        lastIsPlaying = false;
    }
}