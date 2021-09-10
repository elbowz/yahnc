#pragma once

/**
 * TODO:
 * * try to use PROGMEM ?!
 */

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// Units
#define cUnitDegrees "°C"
#define cUnitHpa "hPa"
#define cUnitMgm3 "g/m³"
#define cUnitPercent "%"
#define cUnitTimeMs "ms"
#define cUnitLux "lux"
#define cUnitVolt "V"
#define cUnitAmpere "A"
#define cUnitWatt "W"
#define cUnitCount "#"
#define cUnitMeter "m"
#define cUnitMetersPerSecond "m/s"
#define cUnitMicrosecond "μs"
#define cUnitHz "Hz"

// Topics
#define cStatusTopic "status"
#define cUnitTopic "unit"
#define cTemperatureTopic "temperature"
#define cHeatIndexTopic "heatindex"
#define cTemperatureUnitTopic cTemperatureTopic "/" cUnitTopic
#define cHumidityTopic "humidity"
#define cPressureTopic "pressure"
#define cAbsHumidityTopic "abshumidity"
#define cVoltageTopic "voltage"
#define cBatteryLevelTopic "batterylevel"
#define cDistanceTopic "distance"
#define cPingTopic "ping"
#define cChangedTopic "changed"
#define cValidTopic "valid"