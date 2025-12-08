// SensorManager.h - Temperature sensor manager

#pragma once

#ifndef SENSOR_MANAGER_H
#  define SENSOR_MANAGER_H

#  include <Arduino.h>

#  define INVALID_SENSOR_VALUE INT16_MIN
#  define IS_VALID_SENSOR_VALUE(value) ((value) != INVALID_SENSOR_VALUE)

#  define INVALID_TEMPERATURE_VALUE INT16_MIN
#  define IS_VALID_TEMPERATURE(value) ((value) != INVALID_TEMPERATURE_VALUE)

class DS18B20;

class SensorManager {
 public:
  struct SensorData {
    int16_t temperature;
  };

  SensorManager(DS18B20& sensor, unsigned long intervalMs = 3000);

  void begin();
  void update();

  bool isReady() const;
  SensorData getSensorData() const;

 private:
  enum State { IDLE, REQUESTING, READING };

  DS18B20& sensor;
  State state;
  unsigned long requestTime;
  unsigned long lastReadTime;
  unsigned long interval;
  int16_t lastTemperature;
  bool resultReady;
};

#endif  // SENSOR_MANAGER_H
