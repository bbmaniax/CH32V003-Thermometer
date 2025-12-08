// SSD1306.h - SSD1306 OLED displays driver

#pragma once

#ifndef SSD1306_H
#  define SSD1306_H

#  include <Arduino.h>
#  include <Wire.h>

#  include "Font5x7.h"

#  define SSD1306_BLACK 0
#  define SSD1306_WHITE 1

class SSD1306 {
 public:
  SSD1306(uint8_t width, uint8_t height, uint8_t* buffer, TwoWire* wire = &Wire);

  bool begin(uint8_t address = 0x3C);
  void clearDisplay();
  void display();
  void setRotation(uint8_t rotation);

  uint8_t getWidth() const;
  uint8_t getHeight() const;

  void drawPixel(int16_t x, int16_t y, uint8_t color);
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color = SSD1306_WHITE);
  void drawVLine(int16_t x, int16_t y, int16_t h, uint8_t color = SSD1306_WHITE);
  void drawHLine(int16_t x, int16_t y, int16_t w, uint8_t color = SSD1306_WHITE);

  void drawChar(int16_t x, int16_t y, char c, uint8_t color);
  void setCursor(int16_t x, int16_t y);
  void setTextColor(uint8_t color);
  void setTextColor(uint8_t color, uint8_t bg);
  void setTextSize(uint8_t size);
  void print(const char* str);
  void print(char c);
  void getTextBounds(const char* str, int16_t x, int16_t y, int16_t* x1, int16_t* y1, uint16_t* w, uint16_t* h);

 private:
  void sendCommand(uint8_t cmd);
  void sendCommandList(const uint8_t* cmds, uint8_t count);
  void sendBuffer();

  void drawDiagLine(int16_t x, int16_t y, int16_t len, int8_t xdir, int8_t ydir, uint8_t color = SSD1306_WHITE);

  TwoWire* _wire;
  uint8_t _address;
  uint8_t _width;
  uint8_t _height;
  uint8_t* _buffer;
  int16_t _cursorX;
  int16_t _cursorY;
  uint8_t _textColor;
  uint8_t _textBgColor;
  uint8_t _textSize;
  uint8_t _colOffset;
};

#endif  // SSD1306_H
