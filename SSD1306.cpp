// SSD1306.cpp - SSD1306 OLED displays driver

#include "SSD1306.h"

SSD1306::SSD1306(uint8_t width, uint8_t height, uint8_t* buffer, TwoWire* wire)
    : _wire(wire),
      _address(0x3C),
      _width(width),
      _height(height),
      _buffer(buffer),
      _cursorX(0),
      _cursorY(0),
      _textColor(SSD1306_WHITE),
      _textBgColor(SSD1306_BLACK),
      _textSize(1),
      _colOffset(0) {
}

bool SSD1306::begin(uint8_t address) {
  _address = address;
  _wire->begin();

  if (_width == 96 && _height == 32) {
    _colOffset = 16;
  } else {
    _colOffset = 0;
  }

  uint8_t comPins;
  uint8_t multiplex;
  if (_height == 64) {
    comPins = 0x12;
    multiplex = 0x3F;
  } else if (_height == 16) {
    comPins = 0x02;
    multiplex = 0x0F;
  } else if (_width == 96) {
    comPins = 0x12;
    multiplex = 0x1F;
  } else {
    comPins = 0x02;
    multiplex = 0x1F;
  }

  const uint8_t initCmds[] = {
    0xAE, 0xD5, 0x80,    0xA8, multiplex, 0xD3, 0x00, 0x40, 0x8D, 0x14, 0x20, 0x00, 0xA1,
    0xC8, 0xDA, comPins, 0x81, 0x8F,      0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0xAF,
  };

  sendCommandList(initCmds, sizeof(initCmds));

  clearDisplay();

  return true;
}

void SSD1306::clearDisplay() {
  memset(_buffer, 0, (uint16_t)_width * (_height / 8));
}

void SSD1306::display() {
  sendBuffer();
}

void SSD1306::setRotation(uint8_t rotation) {
  switch (rotation & 3) {
    case 0:
    case 1:
      sendCommand(0xA1);
      sendCommand(0xC8);
      break;
    case 2:
    case 3:
      sendCommand(0xA0);
      sendCommand(0xC0);
      break;
  }
}

uint8_t SSD1306::getWidth() const {
  return _width;
}

uint8_t SSD1306::getHeight() const {
  return _height;
}

void SSD1306::drawPixel(int16_t x, int16_t y, uint8_t color) {
  if (x < 0 || x >= _width || y < 0 || y >= _height) return;

  uint16_t idx = x + (y / 8) * _width;
  uint8_t bit = 1 << (y & 7);
  if (color == SSD1306_WHITE) {
    _buffer[idx] |= bit;
  } else {
    _buffer[idx] &= ~bit;
  }
}

void SSD1306::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color) {
  int16_t dx = abs(x1 - x0);
  int16_t dy = abs(y1 - y0);
  int8_t sx = x0 < x1 ? 1 : -1;
  int8_t sy = y0 < y1 ? 1 : -1;

  if (dx == 0) {
    drawVLine(x0, y0 < y1 ? y0 : y1, dy + 1, color);
    return;
  }

  if (dy == 0) {
    drawHLine(x0 < x1 ? x0 : x1, y0, dx + 1, color);
    return;
  }

  if (dx == dy) {
    drawDiagLine(x0, y0, dx, sx, sy, color);
    return;
  }

  int16_t err = dx - dy;

  while (true) {
    if (x0 >= 0 && x0 < _width && y0 >= 0 && y0 < _height) {
      uint16_t idx = x0 + (y0 / 8) * _width;
      uint8_t bit = 1 << (y0 & 7);
      if (color == SSD1306_WHITE) {
        _buffer[idx] |= bit;
      } else {
        _buffer[idx] &= ~bit;
      }
    }

    if (x0 == x1 && y0 == y1) {
      break;
    }

    int16_t e2 = err * 2;
    if (e2 > -dy) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y0 += sy;
    }
  }
}

void SSD1306::drawVLine(int16_t x, int16_t y, int16_t h, uint8_t color) {
  if (h < 0) {
    y += h + 1;
    h = -h;
  }
  if (x < 0 || x >= _width) return;
  int16_t y1 = y + h - 1;
  if (y1 < 0 && y < 0) return;
  if (y >= _height && y1 >= _height) return;
  if (y < 0) y = 0;
  if (y1 >= _height) y1 = _height - 1;
  for (int16_t yy = y; yy <= y1; yy++) {
    uint16_t idx = x + (yy / 8) * _width;
    uint8_t bit = 1 << (yy & 7);
    if (color == SSD1306_WHITE) {
      _buffer[idx] |= bit;
    } else {
      _buffer[idx] &= ~bit;
    }
  }
}

void SSD1306::drawHLine(int16_t x, int16_t y, int16_t w, uint8_t color) {
  if (w < 0) {
    x += w + 1;
    w = -w;
  }
  if (y < 0 || y >= _height) return;
  int16_t x1 = x + w - 1;
  if (x1 < 0 && x < 0) return;
  if (x >= _width && x1 >= _width) return;
  if (x < 0) x = 0;
  if (x1 >= _width) x1 = _width - 1;
  uint16_t idx = x + (y / 8) * _width;
  uint8_t bit = 1 << (y & 7);
  if (color == SSD1306_WHITE) {
    for (int16_t xx = x; xx <= x1; xx++) {
      _buffer[idx++] |= bit;
    }
  } else {
    uint8_t mask = ~bit;
    for (int16_t xx = x; xx <= x1; xx++) {
      _buffer[idx++] &= mask;
    }
  }
}

void SSD1306::drawChar(int16_t x, int16_t y, char c, uint8_t color) {
  int8_t idx = Font5x7_GetIndex(c);
  if (idx < 0) {
    idx = 0;
  }

  const uint8_t* fontData = &Font5x7[idx * FONT5X7_WIDTH];
  uint8_t bg = _textBgColor;
  uint8_t fontBuffer[FONT5X7_WIDTH];
  memcpy_P(fontBuffer, fontData, FONT5X7_WIDTH);

  if (x < 0 || x + FONT5X7_WIDTH * _textSize > _width) return;
  if (y < 0 || y >= _height) return;

  uint16_t totalHeight = FONT5X7_HEIGHT * _textSize;
  uint8_t startPage = y / 8;
  uint8_t endPage = (y + totalHeight - 1) / 8;

  if (endPage >= _height / 8) {
    endPage = _height / 8 - 1;
  }

  for (uint8_t i = 0; i < FONT5X7_WIDTH; i++) {
    uint8_t line = fontBuffer[i];

    for (uint8_t page = startPage; page <= endPage; page++) {
      uint8_t pageBits = 0;
      uint8_t bgBits = 0;

      for (uint8_t b = 0; b < 8; b++) {
        int16_t screenY = page * 8 + b;
        if (screenY < y || screenY >= y + (int16_t)totalHeight) {
          continue;
        }
        int16_t fontY = screenY - y;
        uint8_t srcBit = fontY / _textSize;
        if (srcBit < FONT5X7_HEIGHT && (line & (1 << srcBit))) {
          pageBits |= (1 << b);
        } else if (srcBit < FONT5X7_HEIGHT) {
          bgBits |= (1 << b);
        }
      }

      uint16_t bufBase = page * _width + x + i * _textSize;
      for (uint8_t col = 0; col < _textSize; col++) {
        uint16_t bufIdx = bufBase + col;
        if (bufIdx >= (uint16_t)_width * (_height / 8)) continue;

        if (bg != color) {
          if (color == SSD1306_WHITE) {
            _buffer[bufIdx] = pageBits;
          } else {
            _buffer[bufIdx] = bgBits;
          }
        } else {
          if (color == SSD1306_WHITE) {
            _buffer[bufIdx] |= pageBits;
          } else {
            _buffer[bufIdx] &= ~pageBits;
          }
        }
      }
    }
  }
}

void SSD1306::setCursor(int16_t x, int16_t y) {
  _cursorX = x;
  _cursorY = y;
}

void SSD1306::setTextColor(uint8_t color) {
  _textColor = color;
  _textBgColor = color;
}

void SSD1306::setTextColor(uint8_t color, uint8_t bg) {
  _textColor = color;
  _textBgColor = bg;
}

void SSD1306::setTextSize(uint8_t size) {
  _textSize = (size > 0) ? size : 1;
}

void SSD1306::print(const char* str) {
  while (*str) {
    print(*str++);
  }
}

void SSD1306::print(char c) {
  if (c == '\n') {
    _cursorX = 0;
    _cursorY += 8 * _textSize;
    return;
  }
  if (c == '\r') {
    _cursorX = 0;
    return;
  }

  drawChar(_cursorX, _cursorY, c, _textColor);
  _cursorX += (FONT5X7_WIDTH + 1) * _textSize;

  if (_cursorX + FONT5X7_WIDTH * _textSize > _width) {
    _cursorX = 0;
    _cursorY += 8 * _textSize;
  }
}

void SSD1306::getTextBounds(const char* str, int16_t x, int16_t y, int16_t* x1, int16_t* y1, uint16_t* w, uint16_t* h) {
  uint16_t len = 0;
  while (str[len]) {
    len++;
  }
  *x1 = x;
  *y1 = y;
  *w = len * (FONT5X7_WIDTH + 1) * _textSize;
  *h = FONT5X7_HEIGHT * _textSize;
}

void SSD1306::sendCommand(uint8_t cmd) {
  _wire->beginTransmission(_address);
  _wire->write(0x00);
  _wire->write(cmd);
  _wire->endTransmission();
}

void SSD1306::sendCommandList(const uint8_t* cmds, uint8_t count) {
  _wire->beginTransmission(_address);
  _wire->write(0x00);
  for (uint8_t i = 0; i < count; i++) {
    _wire->write(cmds[i]);
  }
  _wire->endTransmission();
}

void SSD1306::sendBuffer() {
  sendCommand(0x21);
  sendCommand(_colOffset);
  sendCommand(_colOffset + _width - 1);

  sendCommand(0x22);
  sendCommand(0);
  sendCommand((_height / 8) - 1);

  uint16_t totalBytes = (uint16_t)_width * (_height / 8);

  for (uint16_t i = 0; i < totalBytes; i += 16) {
    _wire->beginTransmission(_address);
    _wire->write(0x40);
    uint8_t chunk = (totalBytes - i > 16) ? 16 : (totalBytes - i);
    for (uint8_t j = 0; j < chunk; j++) {
      _wire->write(_buffer[i + j]);
    }
    _wire->endTransmission();
  }
}

void SSD1306::drawDiagLine(int16_t x, int16_t y, int16_t len, int8_t xdir, int8_t ydir, uint8_t color) {
  for (int16_t i = 0; i <= len; i++) {
    if (x >= 0 && x < _width && y >= 0 && y < _height) {
      uint16_t idx = x + (y / 8) * _width;
      uint8_t bit = 1 << (y & 7);
      if (color == SSD1306_WHITE) {
        _buffer[idx] |= bit;
      } else {
        _buffer[idx] &= ~bit;
      }
    }
    x += xdir;
    y += ydir;
  }
}
