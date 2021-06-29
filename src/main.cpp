#define FW_NAME "multi-sensors-nodes"
#define FW_VERSION "1.0.0"

#ifndef SERIAL_SPEED
#define SERIAL_SPEED 115200
#endif

#include <Homie.h>

#include <BME280Node.hpp>

const int BME280_I2C_ADDRESS = 0x76; // Default I2C address for BME280. can be changed to 0x76 by changing a solder bridge

// Create one node of each kind
BME280Node bme280Node("bme280", "Outdoor", BME280_I2C_ADDRESS);

void setup()
{
  Homie_setFirmware(FW_NAME, FW_VERSION);

  Serial.begin(SERIAL_SPEED);
  Serial << endl
         << endl;

  // Set default configuration values before Homie.setup()
  //bme280Node.beforeHomieSetup();

  // NON SERVE: https://randomnerdtutorials.com/esp8266-bme280-arduino-ide/
  // usando i default:
  const int PIN_SDA = 4;   // =D2 on Wemos
  const int PIN_SCL = 5;   // =D1 on Wemos
  // Initializes I2C for BME280 sensor and display
  Homie.getLogger() << "• Wire begin SDA=" << PIN_SDA << " SCL=" << PIN_SCL << endl;
  Wire.begin(PIN_SDA, PIN_SCL);

  //Homie.disableLedFeedback();
  //Homie.disableResetTrigger();

  Homie.setup();
}

void loop()
{
  Homie.loop();

  // Uncomment for ssl
  // https://github.com/homieiot/homie-esp8266/issues/640
  //delay(100);
  yield();
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

* temp, humiidty, pressure realted calculation:
  https://github.com/finitespace/BME280/blob/master/src/EnvironmentCalculations.h

* platform.io advanced scripting (eg. push config, OTA, etc)
  https://docs.platformio.org/en/latest/projectconf/advanced_scripting.html
*/