// SensorManager.cpp - Temperature sensor manager

#include "SensorManager.h"

#include "DS18B20.h"

SensorManager::SensorManager(DS18B20& sensor, unsigned long intervalMs)
    : sensor(sensor), state(IDLE), requestTime(0), lastReadTime(0), lastTemperature(INVALID_SENSOR_VALUE), resultReady(false) {
  interval = (intervalMs < 750) ? 750 : intervalMs;
}

void SensorManager::begin() {
  sensor.begin();
  state = IDLE;
  requestTime = 0;
  lastTemperature = INVALID_SENSOR_VALUE;
  resultReady = false;
  lastReadTime = millis() - interval;
}

void SensorManager::update() {
  switch (state) {
    case IDLE:
      if (millis() - lastReadTime >= interval) {
        sensor.requestTemparature();
        requestTime = millis();
        state = REQUESTING;
        resultReady = false;
      }
      break;

    case REQUESTING:
      if (millis() - requestTime >= 750) {
        state = READING;
      }
      break;

    case READING: {
      if (!sensor.readTemparature(lastTemperature)) {
        lastTemperature = INVALID_TEMPERATURE_VALUE;
      }
      resultReady = true;
      lastReadTime = millis();
      state = IDLE;
    } break;
  }
}

bool SensorManager::isReady() const {
  bool ready = resultReady;
  if (ready) {
    const_cast<SensorManager*>(this)->resultReady = false;
  }
  return ready;
}

SensorManager::SensorData SensorManager::getSensorData() const {
  SensorData data;
  data.temperature = lastTemperature;
  return data;
}
