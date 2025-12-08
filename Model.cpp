// Model.cpp - Model for Thermometer

#include "Model.h"

#include "SensorDataHistory.h"

Model::Model(SensorDataHistory& temperatureHistory) : temperatureHistory(temperatureHistory) {
}

void Model::begin() {
}

void Model::update(const SensorData& data) {
  temperatureHistory.prepend(data.temperature);
}

int16_t Model::getTemperature() const {
  return temperatureHistory.getValue(0);
}

SensorDataHistory& Model::getTemperatureHistory() const {
  return temperatureHistory;
}
