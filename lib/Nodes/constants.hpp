#pragma once

// TODO:
// * convert to string with F() ?!
// * see if $state topic is used (https://homieiot.github.io/specification/#device-lifecycle) otherwise try to change cStatusTopic

// Units
#define cUnitDegrees "°C"
#define cUnitHpa "hPa"
#define cUnitMgm3 "g/m³"
#define cUnitPercent "%"
#define cUnitTimeMs "ms"
#define cUnitVolt "V"
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