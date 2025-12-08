// View.cpp - View for Thermometer

#include "View.h"

#include "Model.h"
#include "SensorDataHistory.h"
#include "SensorManager.h"
#include "SSD1306.h"

View::View(Model& model, SSD1306& display, uint8_t horizontalStep)
    : model(model), display(display), flipped(false), viewMode(View::VIEW_MODE_CHART), horizontalStep(horizontalStep) {
}

void View::begin() {
  display.begin();
}

void View::render() {
  display.setRotation(flipped ? 2 : 0);
  display.clearDisplay();
  switch (viewMode) {
    case VIEW_MODE_TEXT:
      renderText();
      break;

    case VIEW_MODE_CHART:
      renderChart();
      break;
  }
  display.display();
}

void View::flip() {
  flipped = !flipped;
}

void View::switchToNextViewMode() {
  viewMode = static_cast<ViewMode>((static_cast<int>(viewMode) + 1) % View::VIEW_MODE_COUNT);
}

void View::setViewMode(ViewMode mode) {
  viewMode = mode;
}

void View::renderText() {
  Rect rect = {0, 0, display.getWidth(), display.getHeight()};
  drawSensorData(model.getTemperature(), "C", rect, TEXT_SIZE_LARGE, HALIGN_CENTER, VALIGN_CENTER, false);
}

void View::renderChart() {
  const uint8_t textHeight = 16;
  Rect textRect = {0, 0, display.getWidth(), textHeight};
  Rect rect = {0, textRect.y + textRect.h, display.getWidth(), display.getHeight() - textRect.h};
  drawSensorDataHistory(model.getTemperatureHistory(), rect, horizontalStep);
  drawSensorData(model.getTemperature(), "C", textRect, TEXT_SIZE_MEDIUM, HALIGN_LEFT, VALIGN_TOP, true);
}

void View::drawSensorDataHistory(SensorDataHistory& history, const Rect& rect, uint8_t horizontalStep) {
  if (rect.w <= 0 || rect.h <= 0 || horizontalStep == 0) {
    return;
  }

  const int16_t chartX = rect.x;
  const int16_t chartY = rect.y;
  const int16_t chartW = rect.w;
  const int16_t chartH = rect.h;

  size_t count = history.getCount();

  if (count >= 2) {
    size_t maxDataPoints = (chartW + horizontalStep - 1) / horizontalStep + 1;
    size_t drawCount = count < maxDataPoints ? count : maxDataPoints;

    int16_t minValue, maxValue;
    history.getMinMaxValue(drawCount, minValue, maxValue);

    if (!IS_VALID_TEMPERATURE(minValue) || !IS_VALID_TEMPERATURE(maxValue)) {
      return;
    }

    int16_t range = maxValue - minValue;

    for (size_t i = 0; i < drawCount - 1; i++) {
      int16_t currentValue = history.getValue(i);
      int16_t nextValue = history.getValue(i + 1);

      if (IS_VALID_TEMPERATURE(currentValue) && IS_VALID_TEMPERATURE(nextValue)) {
        int16_t currentY, nextY;

        if (range == 0) {
          currentY = chartY + chartH / 2;
          nextY = chartY + chartH / 2;
        } else {
          currentY = chartY + (int16_t)(((int32_t)(maxValue - currentValue) * (chartH - 1)) / range);
          nextY = chartY + (int16_t)(((int32_t)(maxValue - nextValue) * (chartH - 1)) / range);
        }

        int16_t currentX = chartX + chartW - 1 - (i * horizontalStep);
        int16_t nextX = chartX + chartW - 1 - ((i + 1) * horizontalStep);
        display.drawLine(currentX, currentY, nextX, nextY);
      }
    }
  }
}

void View::drawSensorData(int16_t value, const char* unit, const Rect& rect, TextSize textSize, HorizontalAlign hAlign, VerticalAlign vAlign, bool withBackground) {
  static char valueTextBuffer[8];
  static char unitTextBuffer[4];

  strcpy(valueTextBuffer, "--.-");
  if (IS_VALID_TEMPERATURE(value)) {
    int16_t intPart = value / 100;
    uint8_t fracPart = abs(value % 100) / 10;
    if (value < 0 && intPart == 0) {
      sprintf(valueTextBuffer, "-0.%u", fracPart);
    } else {
      sprintf(valueTextBuffer, "%d.%u", intPart, fracPart);
    }
  }

  TextSize valueSize = textSize;
  TextSize unitSize = (textSize >= TEXT_SIZE_MEDIUM) ? static_cast<TextSize>(textSize - 1) : textSize;

  // Add degree symbol for temperature
  if (strcmp(unit, "C") == 0) {
    strcpy(unitTextBuffer, "\001C");
  } else {
    strncpy(unitTextBuffer, unit, sizeof(unitTextBuffer) - 1);
    unitTextBuffer[sizeof(unitTextBuffer) - 1] = '\0';
  }

  int16_t x1, y1;
  uint16_t valueW, valueH, unitW, unitH;

  display.setTextSize(valueSize);
  display.getTextBounds(valueTextBuffer, 0, 0, &x1, &y1, &valueW, &valueH);

  display.setTextSize(unitSize);
  display.getTextBounds(unitTextBuffer, 0, 0, &x1, &y1, &unitW, &unitH);

  uint16_t totalW = valueW + unitW;
  uint16_t totalH = valueH;

  int16_t cursorX = rect.x;
  int16_t cursorY = rect.y;

  switch (hAlign) {
    case HALIGN_LEFT:
      break;
    case HALIGN_CENTER:
      cursorX += (rect.w - totalW) / 2;
      break;
    case HALIGN_RIGHT:
      cursorX += rect.w - totalW;
      break;
  }

  switch (vAlign) {
    case VALIGN_TOP:
      break;
    case VALIGN_CENTER:
      cursorY += (rect.h - totalH) / 2;
      break;
    case VALIGN_BOTTOM:
      cursorY += rect.h - totalH;
      break;
  }

  if (withBackground) {
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  } else {
    display.setTextColor(SSD1306_WHITE);
  }

  display.setTextSize(valueSize);
  display.setCursor(cursorX, cursorY);
  display.print(valueTextBuffer);

  display.setTextSize(unitSize);
  display.setCursor(cursorX + valueW, cursorY);
  display.print(unitTextBuffer);
}
