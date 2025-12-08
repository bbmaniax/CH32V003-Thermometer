// DS18B20.h - DS18B20 temperature sensor driver

#pragma once

#ifndef DS18B20_H
#  define DS18B20_H

#  include <Arduino.h>

class OneWire;

class DS18B20 {
 public:
  DS18B20(OneWire& wire, int16_t offset = 0);

  void begin(void);
  void requestTemparature(void);
  bool readTemparature(int16_t& temperature);

 private:
  OneWire& wire;
  int16_t offset;
};

#endif  // DS18B20_H
