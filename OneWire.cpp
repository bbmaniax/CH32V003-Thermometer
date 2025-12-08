// OneWire.cpp - Arduino library for 1-Wire communication

#include "OneWire.h"

#if defined(__AVR__)
#  define OW_DISABLE_IRQ() noInterrupts()
#  define OW_ENABLE_IRQ() interrupts()
static inline void delay_us(uint16_t us) {
  delayMicroseconds(us);
}

static inline void ow_output_low(uint8_t pin) {
  digitalWrite(pin, LOW);
  pinMode(pin, OUTPUT);
}

static inline void ow_input(uint8_t pin, bool useInputPullup) {
  if (useInputPullup) {
    digitalWrite(pin, HIGH);
    pinMode(pin, OUTPUT);
    pinMode(pin, INPUT_PULLUP);
  } else {
    pinMode(pin, INPUT);
  }
}

static inline uint8_t ow_read(uint8_t pin) {
  return digitalRead(pin) ? 1 : 0;
}
#elif defined(__riscv) && defined(CH32V003)
#  include "ch32v00x.h"

extern uint32_t SystemCoreClock;
#  define OW_DISABLE_IRQ()
#  define OW_ENABLE_IRQ()

static void delay_us(uint16_t us) __attribute__((noinline));

static void delay_us(uint16_t us) {
  uint32_t cycles = (uint32_t)us * (SystemCoreClock / 4000000);
  __asm__ volatile(
    "1: addi %0, %0, -1\n"
    "   bnez %0, 1b\n"
    : "+r"(cycles));
}

static inline GPIO_TypeDef* ow_get_gpio(uint8_t pin) {
  uint8_t port = (pin & CH32_GPIO_MASK) >> 5;
  switch (port) {
    case 1:
      return GPIOA;
    case 4:
      return GPIOD;
    default:
      return GPIOC;
  }
}

static inline void ow_output_low(uint8_t pin) {
  GPIO_TypeDef* gpio = ow_get_gpio(pin);
  uint8_t pinNum = pin & CH32_PIN_MASK;
  uint8_t shift = pinNum * 4;
  gpio->BSHR = (1 << (pinNum + 16));
  gpio->CFGLR = (gpio->CFGLR & ~(0xFUL << shift)) | (0x3UL << shift);
}

static inline void ow_input(uint8_t pin, bool useInputPullup) {
  GPIO_TypeDef* gpio = ow_get_gpio(pin);
  uint8_t pinNum = pin & CH32_PIN_MASK;
  uint8_t shift = pinNum * 4;
  if (useInputPullup) {
    gpio->BSHR = (1 << pinNum);
    gpio->CFGLR = (gpio->CFGLR & ~(0xFUL << shift)) | (0x3UL << shift);
    gpio->CFGLR = (gpio->CFGLR & ~(0xFUL << shift)) | (0x8UL << shift);
  } else {
    gpio->CFGLR = (gpio->CFGLR & ~(0xFUL << shift)) | (0x4UL << shift);
  }
}

static inline uint8_t ow_read(uint8_t pin) {
  GPIO_TypeDef* gpio = ow_get_gpio(pin);
  uint8_t pinNum = pin & CH32_PIN_MASK;
  return (gpio->INDR & (1 << pinNum)) ? 1 : 0;
}
#else
#  error "Not supported"
#endif

OneWire::OneWire(uint8_t pin, bool useInputPullup) : pin(pin), useInputPullup(useInputPullup) {
}

void OneWire::begin(void) {
  ow_input(pin, useInputPullup);
}

uint8_t OneWire::reset(void) {
  uint8_t r;
  uint8_t retries = 125;

  OW_DISABLE_IRQ();

  ow_input(pin, useInputPullup);
  while (ow_read(pin) == 0) {
    if (--retries == 0) {
      OW_ENABLE_IRQ();
      return 0;
    }
    delay_us(2);
  }

  ow_output_low(pin);
  delay_us(480);

  ow_input(pin, useInputPullup);
  delay_us(70);
  r = (ow_read(pin) == 0) ? 1 : 0;
  delay_us(410);

  OW_ENABLE_IRQ();
  return r;
}

void OneWire::skip(void) {
  write(0xCC);
}

void OneWire::write_bit(uint8_t v) {
  OW_DISABLE_IRQ();
  if (v & 1) {
    ow_output_low(pin);
    delay_us(10);
    ow_input(pin, useInputPullup);
    delay_us(55);
  } else {
    ow_output_low(pin);
    delay_us(65);
    ow_input(pin, useInputPullup);
    delay_us(5);
  }
  OW_ENABLE_IRQ();
}

uint8_t OneWire::read_bit(void) {
  uint8_t r;

  OW_DISABLE_IRQ();
  ow_output_low(pin);
  delay_us(1);
  ow_input(pin, useInputPullup);
  r = ow_read(pin);
  delay_us(60);
  OW_ENABLE_IRQ();

  return r;
}

void OneWire::write(uint8_t v, uint8_t power) {
  for (uint8_t bitMask = 0x01; bitMask; bitMask <<= 1) {
    write_bit((bitMask & v) ? 1 : 0);
  }
  if (!power) {
    ow_input(pin, useInputPullup);
  }
}

uint8_t OneWire::read(void) {
  uint8_t r = 0;
  for (uint8_t bitMask = 0x01; bitMask; bitMask <<= 1) {
    if (read_bit()) {
      r |= bitMask;
    }
  }
  return r;
}
