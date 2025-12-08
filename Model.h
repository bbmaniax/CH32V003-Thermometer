// Model.h - Model for Thermometer

#pragma once

#ifndef MODEL_H
#  define MODEL_H

#  include <Arduino.h>

#  include "SensorManager.h"

class SensorDataHistory;

class Model {
 public:
  using SensorData = SensorManager::SensorData;

  Model(SensorDataHistory& temperatureHistory);

  void begin();
  void update(const SensorData& data);

  int16_t getTemperature() const;
  SensorDataHistory& getTemperatureHistory() const;

 private:
  SensorDataHistory& temperatureHistory;
};

#endif  // MODEL_H
