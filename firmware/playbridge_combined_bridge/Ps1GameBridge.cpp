#include "Ps1GameBridge.h"

#include <WiFi.h>
#include <WiFiUdp.h>
#include <atomic>
#include <cerrno>
#include <cstring>
#include <driver/uart.h>
#include <lwip/sockets.h>

namespace Ps1GameBridge {
namespace {

constexpr uint16_t kDataPort = 3333;
constexpr uint16_t kControlPort = 3334;
constexpr uint32_t kInitialBaud = 115200;
constexpr uint32_t kStallTimeoutMs = 10000;
constexpr uint32_t kBaudDrainTimeoutMs = 150;
constexpr size_t kTcpPendingSize = 16384;
constexpr size_t kUartPendingSize = 1024;

HardwareSerial serial2(2);
WiFiServer dataServer(kDataPort);
WiFiClient dataClient;
WiFiUDP controlSocket;

uint8_t tcpPending[kTcpPendingSize];
size_t tcpPendingSize = 0;
size_t tcpPendingOffset = 0;
uint8_t uartPending[kUartPendingSize];
size_t uartPendingSize = 0;
size_t uartPendingOffset = 0;

Status live{};
Status published{};
portMUX_TYPE statusMux = portMUX_INITIALIZER_UNLOCKED;
std::atomic<bool> active{false};
std::atomic<uint32_t> asyncUartErrors{0};
std::atomic<int32_t> asyncLastUartError{0};
uint32_t sessionUartErrors = 0;
uint32_t lastProgressMs = 0;
uint32_t previousPollUs = 0;

struct ControlCache {
  bool valid = false;
  IPAddress ip;
  uint8_t packet[20]{};
};
ControlCache lastControl;

bool supportedBaud(uint32_t baud) {
  return baud == 115200 || baud == 230400 || baud == 518400 ||
         baud == 691200 || baud == 1036800 || baud == 2073600;
}

uint32_t crc32(const uint8_t *data, size_t length) {
  uint32_t crc = 0xffffffffu;
  while (length--) {
    crc ^= *data++;
    for (int bit = 0; bit < 8; ++bit) {
      crc = (crc >> 1) ^ (0xedb88320u & (0u - (crc & 1u)));
    }
  }
  return ~crc;
}

void publishAsyncErrors() {
  live.uartErrors = asyncUartErrors.load(std::memory_order_relaxed);
  live.lastUartError = asyncLastUartError.load(std::memory_order_relaxed);
}

void publishStatus() {
  portENTER_CRITICAL(&statusMux);
  published = live;
  portEXIT_CRITICAL(&statusMux);
}

void clearQueues() {
  tcpPendingSize = tcpPendingOffset = 0;
  uartPendingSize = uartPendingOffset = 0;
}

void abortSession() {
  live.abortedBytes += static_cast<uint32_t>(tcpPendingSize - tcpPendingOffset);
  live.abortedBytes += static_cast<uint32_t>(uartPendingSize - uartPendingOffset);
  clearQueues();
  dataClient.stop();
  live.sessionActive = false;
  active.store(false, std::memory_order_release);
}

bool sameControlRequest(const uint8_t packet[20], const IPAddress &ip) {
  return lastControl.valid && lastControl.ip == ip &&
         memcmp(lastControl.packet, packet, sizeof(lastControl.packet)) == 0;
}

bool sendControlAck(const uint8_t packet[20], const IPAddress &ip,
                    uint16_t port) {
  if (!controlSocket.beginPacket(ip, port)) return false;
  if (controlSocket.write(packet, 20) != 20) return false;
  if (!controlSocket.endPacket()) return false;
  ++live.controlAcks;
  return true;
}

bool uartDrained() {
  if (uartPendingOffset != uartPendingSize || dataClient.available() > 0) {
    return false;
  }
  return uart_wait_tx_done(UART_NUM_2,
                           pdMS_TO_TICKS(kBaudDrainTimeoutMs)) == ESP_OK;
}

void handleControl() {
  const int packetSize = controlSocket.parsePacket();
  if (packetSize <= 0) return;

  const IPAddress sender = controlSocket.remoteIP();
  const uint16_t senderPort = controlSocket.remotePort();
  uint8_t packet[20]{};
  if (packetSize != 20 || controlSocket.read(packet, sizeof(packet)) != 20) {
    while (controlSocket.available()) controlSocket.read();
    ++live.malformedControl;
    return;
  }

  if (sameControlRequest(packet, sender)) {
    // The host retries identical packets. Re-acknowledge without repeating an
    // abort that could otherwise kill a newly established data session.
    ++live.duplicateControl;
    sendControlAck(packet, sender, senderPort);
    return;
  }

  uint32_t baud = 0;
  uint32_t flags = 0;
  uint32_t receivedCrc = 0;
  memcpy(&baud, packet + 8, sizeof(baud));
  memcpy(&flags, packet + 12, sizeof(flags));
  memcpy(&receivedCrc, packet + 16, sizeof(receivedCrc));

  if (memcmp(packet, "PSB1", 4) != 0 || receivedCrc != crc32(packet, 16) ||
      flags > 1 || !supportedBaud(baud)) {
    ++live.malformedControl;
    return;
  }
  if (live.sessionActive && sender != dataClient.remoteIP()) {
    ++live.malformedControl;
    return;
  }

  if (flags == 1) abortSession();
  if (!uartDrained()) return;  // No success ACK when the handoff is unsafe.

  if (live.baud != baud) {
    serial2.updateBaudRate(baud);
    live.baud = baud;
  }

  if (sendControlAck(packet, sender, senderPort)) {
    lastControl.valid = true;
    lastControl.ip = sender;
    memcpy(lastControl.packet, packet, sizeof(lastControl.packet));
  }
}

void acceptClient() {
  WiFiClient candidate = dataServer.accept();
  if (!candidate) return;
  if (live.sessionActive) {
    ++live.rejectedClients;
    candidate.stop();
    return;
  }

  dataClient = candidate;
  dataClient.setNoDelay(true);
  clearQueues();
  ++live.sessions;
  live.sessionActive = true;
  active.store(true, std::memory_order_release);
  lastProgressMs = millis();
  sessionUartErrors = asyncUartErrors.load(std::memory_order_relaxed);
}

void serviceTcpToUart() {
  if (uartPendingOffset == uartPendingSize) {
    uartPendingOffset = uartPendingSize = 0;
    const int count = min(dataClient.available(),
                          static_cast<int>(sizeof(uartPending)));
    if (count > 0) {
      const int received = dataClient.read(uartPending, count);
      if (received > 0) {
        uartPendingSize = static_cast<size_t>(received);
        live.tcpReceived += static_cast<uint32_t>(received);
      }
    }
  }

  if (uartPendingOffset >= uartPendingSize) return;
  const size_t writable = static_cast<size_t>(max(0, serial2.availableForWrite()));
  const size_t count = min(writable, uartPendingSize - uartPendingOffset);
  if (!count) return;

  const size_t written = serial2.write(uartPending + uartPendingOffset, count);
  uartPendingOffset += written;
  live.uartWritten += static_cast<uint32_t>(written);
  if (written) lastProgressMs = millis();
}

void serviceUartToTcp() {
  if (tcpPendingOffset == tcpPendingSize) {
    tcpPendingOffset = tcpPendingSize = 0;
  } else if (tcpPendingSize == sizeof(tcpPending) && tcpPendingOffset) {
    memmove(tcpPending, tcpPending + tcpPendingOffset,
            tcpPendingSize - tcpPendingOffset);
    tcpPendingSize -= tcpPendingOffset;
    tcpPendingOffset = 0;
  }

  const int serialAvailable = serial2.available();
  live.uartRxPeak = max(live.uartRxPeak, static_cast<uint32_t>(serialAvailable));
  int count = min(serialAvailable,
                  static_cast<int>(sizeof(tcpPending) - tcpPendingSize));
  count = min(count, 1024);
  while (count-- > 0) {
    const int value = serial2.read();
    if (value < 0) break;
    tcpPending[tcpPendingSize++] = static_cast<uint8_t>(value);
    ++live.uartReceived;
  }
  live.pendingPeak = max(
      live.pendingPeak, static_cast<uint32_t>(tcpPendingSize - tcpPendingOffset));

  if (tcpPendingOffset >= tcpPendingSize) return;
  const size_t countToSend = min(static_cast<size_t>(1460),
                                 tcpPendingSize - tcpPendingOffset);
  const int sent = lwip_send(dataClient.fd(), tcpPending + tcpPendingOffset,
                             countToSend, MSG_DONTWAIT);
  if (sent > 0) {
    tcpPendingOffset += static_cast<size_t>(sent);
    live.tcpWritten += static_cast<uint32_t>(sent);
    lastProgressMs = millis();
  } else if (sent < 0 && errno != EAGAIN && errno != EWOULDBLOCK &&
             errno != EINTR) {
    ++live.socketErrors;
    abortSession();
  }
}

}  // namespace

bool begin(int rxPin, int txPin) {
  if (live.initialized || rxPin == txPin || rxPin < 0 || txPin < 0) return false;

  serial2.setRxBufferSize(16384);
  serial2.setTxBufferSize(4096);
  serial2.begin(kInitialBaud, SERIAL_8N1, rxPin, txPin);
  serial2.onReceiveError([](hardwareSerial_error_t error) {
    asyncUartErrors.fetch_add(1, std::memory_order_relaxed);
    asyncLastUartError.store(static_cast<int32_t>(error),
                             std::memory_order_relaxed);
  });
  if (!controlSocket.begin(kControlPort)) {
    serial2.end();
    return false;
  }

  dataServer.begin();
  dataServer.setNoDelay(true);
  live.baud = kInitialBaud;
  live.initialized = true;
  previousPollUs = micros();
  publishStatus();
  return true;
}

void poll() {
  if (!live.initialized) {
    vTaskDelay(1);
    return;
  }

  const uint32_t startedUs = micros();
  const uint32_t serviceGapUs = startedUs - previousPollUs;
  previousPollUs = startedUs;
  live.maxServiceGapUs = max(live.maxServiceGapUs, serviceGapUs);
  publishAsyncErrors();
  handleControl();

  if (!live.sessionActive) {
    int discardBudget = 512;
    while (discardBudget-- > 0 && serial2.available()) {
      serial2.read();
      ++live.idleDiscarded;
    }
    acceptClient();
  } else {
    if (asyncUartErrors.load(std::memory_order_relaxed) != sessionUartErrors) {
      abortSession();
    } else {
      acceptClient();  // Reject any additional client without disturbing owner.
      serviceTcpToUart();
      serviceUartToTcp();

      if (!dataClient.connected() && !dataClient.available() &&
          uartPendingOffset == uartPendingSize) {
        abortSession();
      } else if ((tcpPendingOffset < tcpPendingSize ||
                  uartPendingOffset < uartPendingSize) &&
                 millis() - lastProgressMs > kStallTimeoutMs) {
        ++live.socketErrors;
        abortSession();
      }
    }
  }

  live.maxIterationUs = max(live.maxIterationUs, micros() - startedUs);
  publishStatus();
  vTaskDelay(1);
}

Status snapshot() {
  portENTER_CRITICAL(&statusMux);
  const Status copy = published;
  portEXIT_CRITICAL(&statusMux);
  return copy;
}

bool transferActive() { return active.load(std::memory_order_acquire); }

}  // namespace Ps1GameBridge
