#pragma once

#include <Arduino.h>

namespace Ps1GameBridge {

struct Status {
  bool initialized;
  bool sessionActive;
  uint32_t baud;
  uint32_t sessions;
  uint32_t tcpReceived;
  uint32_t uartWritten;
  uint32_t uartReceived;
  uint32_t tcpWritten;
  uint32_t uartErrors;
  int32_t lastUartError;
  uint32_t socketErrors;
  uint32_t abortedBytes;
  uint32_t rejectedClients;
  uint32_t malformedControl;
  uint32_t duplicateControl;
  uint32_t controlAcks;
  uint32_t pendingPeak;
  uint32_t uartRxPeak;
  uint32_t maxServiceGapUs;
  uint32_t maxIterationUs;
  uint32_t idleDiscarded;
};

// Call begin() and poll() from one dedicated task. No other task may access
// UART2, TCP port 3333, or UDP port 3334.
bool begin(int rxPin = 16, int txPin = 17);
void poll();
Status snapshot();
bool transferActive();

}  // namespace Ps1GameBridge
