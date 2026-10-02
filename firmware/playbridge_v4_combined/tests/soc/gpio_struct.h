#pragma once
#include "Arduino.h"
struct ReadLow { operator uint32_t() const { return mockLow(); } };
struct ReadHigh { operator uint32_t() const { return mockHigh(); } };
struct WriteReg { int kind; void operator=(uint32_t v) { mockWrite(kind, v); } };
struct MockGPIO {
  ReadLow in;
  struct { ReadHigh val; } in1;
  struct { WriteReg val{1}; } out1_w1ts;
  struct { WriteReg val{2}; } out1_w1tc;
  WriteReg out_w1ts{3}, out_w1tc{4};
};
inline MockGPIO GPIO;
