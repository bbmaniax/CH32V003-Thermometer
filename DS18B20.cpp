// DS18B20.cpp - DS18B20 temperature sensor driver

#include "DS18B20.h"

#include "OneWire.h"
#include "SensorManager.h"

DS18B20::DS18B20(OneWire& wire, int16_t offset) : wire(wire), offset(offset) {
}

void DS18B20::begin(void) {
  wire.begin();
}

void DS18B20::requestTemparature(void) {
  wire.reset();
  wire.skip();
  wire.write(0x44, 0);
}

bool DS18B20::readTemparature(int16_t& temperature) {
  wire.reset();
  wire.skip();
  wire.write(0xBE, 0);

  uint8_t data[9];
  for (int i = 0; i < 9; i++) {
    data[i] = wire.read();
  }

  if (data[0] == 0xFF && data[1] == 0xFF) {
    return false;
  }

  int16_t raw = (data[1] << 8) | data[0];
  int32_t temparature = (static_cast<int32_t>(raw) * 100) / 16;
  temperature = static_cast<int16_t>(temparature) + offset;
  return true;
}
