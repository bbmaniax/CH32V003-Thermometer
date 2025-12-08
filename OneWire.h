// OneWire.h - Arduino library for 1-Wire communication

#pragma once

#ifndef ONEWIRE_H
#  define ONEWIRE_H

#  include <Arduino.h>

class OneWire {
 public:
  OneWire(uint8_t pin, bool useInputPullup);

  void begin(void);
  uint8_t reset(void);
  void skip(void);
  void write(uint8_t v, uint8_t power = 0);
  uint8_t read(void);
  void write_bit(uint8_t v);
  uint8_t read_bit(void);

 private:
  uint8_t pin;
  bool useInputPullup;
};

#endif  // ONEWIRE_H
