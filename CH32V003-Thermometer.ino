// CH32V003-Thermometer.ino - Main sketch for CH32V003-based thermometer

#include <Arduino.h>
#include <DigitalButton.h>

#include "DS18B20.h"
#include "Model.h"
#include "OneWire.h"
#include "SSD1306.h"
#include "SensorDataHistory.h"
#include "SensorManager.h"
#include "View.h"

#define SERIAL_SPEED 115200
#define BUTTON_PIN PD0
#define DS18B20_PIN PC5
#define DS18B20_TEMPERATURE_OFFSET -9
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 32
#define DISPLAY_BUFFER_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT / 8)
#define MEASUREMENT_INTERVAL_MS 3000
#define HORIZONTAL_STEP 3
#define HISTORY_BUFFER_SIZE ((DISPLAY_WIDTH + HORIZONTAL_STEP - 1) / HORIZONTAL_STEP + 1)

uint8_t displayBuffer[DISPLAY_BUFFER_SIZE];
int16_t temperatureHistoryBuffer[HISTORY_BUFFER_SIZE];

DigitalButton button(BUTTON_PIN, true);
SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, displayBuffer);
OneWire oneWire(DS18B20_PIN, true);
DS18B20 ds18b20(oneWire, DS18B20_TEMPERATURE_OFFSET);

SensorDataHistory temperatureHistory(temperatureHistoryBuffer, HISTORY_BUFFER_SIZE);
Model model(temperatureHistory);
View view(model, display, HORIZONTAL_STEP);
SensorManager sensorManager(ds18b20, MEASUREMENT_INTERVAL_MS);

void setup() {
  // Serial.begin(SERIAL_SPEED);
  // while (!Serial && millis() < 1000);
  Serial.println();
  Serial.println("--");
  Serial.println("Thermometer (built at " __DATE__ " " __TIME__ ")");

  button.begin();
  oneWire.begin();
  sensorManager.begin();
  delay(100);

  model.begin();
  view.begin();
}

void loop() {
  static bool needRender = true;

  button.update();
  sensorManager.update();

  if (button.isLongPressed()) {
    view.flip();
    needRender = true;
  }

  if (button.isClicked()) {
    view.switchToNextViewMode();
    needRender = true;
  }

  if (sensorManager.consumeReady()) {
    SensorManager::SensorData data = sensorManager.getSensorData();
    printSensorData(millis(), data);
    model.update(data);
    needRender = true;
  }

  if (needRender) {
    view.render();
    needRender = false;
  }

  delay(10);
}

void printSensorData(unsigned long timestamp, const SensorManager::SensorData& data) {
  Serial.print("TS:");
  Serial.print(timestamp);
  Serial.print(" T:");
  printSensorValue(data.temperature);
  Serial.println();
}

void printSensorValue(int16_t value) {
  int16_t intPart = value / 10;
  uint8_t fracPart = abs(value % 10);

  if (value < 0 && intPart == 0) {
    Serial.print("-0.");
  } else {
    Serial.print(intPart);
    Serial.print(".");
  }

  Serial.print(fracPart);
}
