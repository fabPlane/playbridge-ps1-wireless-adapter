#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#define IRAM_ATTR
#define LOW 0
#define HIGH 1
#define INPUT 0
#define OUTPUT 1
using portMUX_TYPE = int;
#define portMUX_INITIALIZER_UNLOCKED 0
#define portENTER_CRITICAL(x) ((void)0)
#define portEXIT_CRITICAL(x) ((void)0)
uint32_t mockCycles();
uint32_t mockLow();
uint32_t mockHigh();
void mockWrite(int kind, uint32_t value);
void mockDelay(uint32_t us);
inline void digitalWrite(int pin, int v) { mockWrite(pin == 33 ? (v ? 1 : 2) : (v ? 3 : 4), 0); }
inline void pinMode(int, int) {}
inline int digitalRead(int pin) { return pin < 32 ? (mockLow() >> pin) & 1 : (mockHigh() >> (pin - 32)) & 1; }
inline void delay(int ms) { mockDelay(ms * 1000); }
inline uint32_t millis() { return mockCycles() / 240000; }
inline uint32_t getCpuFrequencyMhz() { return 240; }
struct SerialMock {
  void begin(int) {}
  int available() { return 0; }
  int read() { return 0; }
  template <typename... T> void printf(const char*, T...) {}
  void print(const char*) {}
  void println(const char* = "") {}
};
inline SerialMock Serial;
