// View.h - View for Thermometer

#pragma once

#ifndef VIEW_H
#  define VIEW_H

#  include <Arduino.h>

class Model;
class SSD1306;
class SensorDataHistory;

class View {
 public:
  struct Rect {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
  };

  enum ViewMode {
    VIEW_MODE_CHART = 0,
    VIEW_MODE_TEXT,
    VIEW_MODE_COUNT,
  };

  enum HorizontalAlign {
    HALIGN_LEFT,
    HALIGN_CENTER,
    HALIGN_RIGHT,
  };

  enum VerticalAlign {
    VALIGN_TOP,
    VALIGN_CENTER,
    VALIGN_BOTTOM,
  };

  enum TextSize {
    TEXT_SIZE_SMALL = 1,
    TEXT_SIZE_MEDIUM = 2,
    TEXT_SIZE_LARGE = 3,
  };

  View(Model& model, SSD1306& display, uint8_t horizontalStep = 1);

  void begin();
  void render();
  void flip();
  void switchToNextViewMode();
  void setViewMode(ViewMode mode);

 private:
  void renderText();
  void renderChart();
  void drawSensorData(int16_t value, const char* unit, const Rect& rect, TextSize textSize, HorizontalAlign hAlign, VerticalAlign vAlign, bool withBackground);
  void drawSensorDataHistory(SensorDataHistory& history, const Rect& rect, uint8_t horizontalStep);

  Model& model;
  SSD1306& display;
  uint8_t horizontalStep;
  ViewMode viewMode;
  bool flipped;
};

#endif  // VIEW_H
