// PlayBridge V3 combined experimental firmware.
//
// The validated controller source remains untouched in its original directory.
// It is included here under private compatibility names so this integration can
// share its proven low-level SPI engine, HTML assets, and diagnostics without
// maintaining a divergent copy.
#define setup playbridgeBaseSetup
#define loop playbridgeBaseLoop
#define beginPs1Spi playbridgeBaseBeginPs1Spi
#define onPs1SpiByte playbridgeBaseOnPs1SpiByte
#define onAttentionRising playbridgeBaseOnAttentionRising
#define setPadInput playbridgeBaseSetPadInput
#define setButtonState playbridgeBaseSetButtonState
#define handleRoot playbridgeBaseHandleRoot
#define handleInput playbridgeBaseHandleInput
#define handleButton playbridgeBaseHandleButton
#define handleSerialSetup playbridgeBaseHandleSerialSetup
#define connectToLocalWifi playbridgeBaseConnectToLocalWifi
#define connectUsingSavedWifi playbridgeBaseConnectUsingSavedWifi
#include "../playbridge_wifi_test/playbridge_wifi_test.ino"
#undef setup
#undef loop
#undef beginPs1Spi
#undef onPs1SpiByte
#undef onAttentionRising
#undef setPadInput
#undef setButtonState
#undef handleRoot
#undef handleInput
#undef handleButton
#undef handleSerialSetup
#undef connectToLocalWifi
#undef connectUsingSavedWifi

#include "Ps1GameBridge.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace {

constexpr char kFirmwareVersion[] = "playbridge-v3-combined-0.1.0";
constexpr uint32_t kControllerExpiryMs = 5000;
constexpr uint32_t kWifiConnectTimeoutMs = 15000;
constexpr BaseType_t kBridgeCore = 0;
constexpr uint32_t kBridgeStackBytes = 8192;

portMUX_TYPE padMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint16_t latchedButtons = 0xffff;
uint32_t lastControllerHeartbeatMs = 0;
uint32_t wifiConnectStartedMs = 0;
bool wifiConnectPending = false;
TaskHandle_t bridgeTaskHandle = nullptr;

char serialLine[96]{};
size_t serialLineLength = 0;
bool awaitingWifiPassword = false;

uint16_t padSnapshot() {
  portENTER_CRITICAL(&padMux);
  const uint16_t value = psButtons;
  portEXIT_CRITICAL(&padMux);
  return value;
}

void setPadMask(uint16_t value) {
  portENTER_CRITICAL(&padMux);
  psButtons = value;
  portEXIT_CRITICAL(&padMux);
}

bool setButtonState(const String &name, bool pressed) {
  const int8_t bit = buttonBit(name);
  if (bit < 0) return false;

  portENTER_CRITICAL(&padMux);
  if (pressed) {
    psButtons &= ~(1U << bit);
  } else {
    psButtons |= 1U << bit;
  }
  portEXIT_CRITICAL(&padMux);
  lastInput = pressed ? name : "RELEASE " + name;
  lastControllerHeartbeatMs = millis();
  return true;
}

void setPadInput(const String &input) {
  uint16_t value = 0xffff;
  if (input == "UP") value &= ~(1U << 4);
  else if (input == "RIGHT") value &= ~(1U << 5);
  else if (input == "DOWN") value &= ~(1U << 6);
  else if (input == "LEFT") value &= ~(1U << 7);
  else if (input == "CROSS") value &= ~(1U << 14);
  setPadMask(value);
  lastControllerHeartbeatMs = millis();
}

String combinedPadReplyText() {
  const uint16_t buttons = padSnapshot();
  char reply[15];
  snprintf(reply, sizeof(reply), "FF 41 5A %02X %02X", buttons & 0xff,
           buttons >> 8);
  return String(reply);
}

void IRAM_ATTR onAttentionRising() {
  portENTER_CRITICAL_ISR(&padMux);
  latchedButtons = psButtons;
  portEXIT_CRITICAL_ISR(&padMux);
  psControllerSelected = false;
  psByteIndex = 0;
  releaseData();
  fastDrive(ACK_DRIVE_PIN, false);
  armPs1Byte(0xff);
}

void IRAM_ATTR onPs1SpiByte(void *) {
  esp_rom_delay_us(14);

  const uint8_t completed = psByteIndex;
  const uint8_t received = static_cast<uint8_t>(SPI2.data_buf[0]);
  ++psSpiByteCount;
  if (completed < sizeof(psLastRx)) psLastRx[completed] = received;

  uint8_t nextReply = 0xff;
  if (completed == 0) {
    ++psAttFallingCount;
    psLastAddress = received;
    psControllerSelected = received == 0x01;
    if (psControllerSelected) {
      ++psAddressMatchCount;
      pinMatrixOutAttach(DATA_DRIVE_PIN, HSPIQ_OUT_IDX, false, false);
      nextReply = 0x41;
    }
  } else if (psControllerSelected) {
    if (completed == 1) {
      psLastCommand = received;
      if (received == 0x42) ++psCommand42Count;
      nextReply = 0x5a;
    } else if (completed == 2) {
      nextReply = static_cast<uint8_t>(latchedButtons & 0xff);
    } else if (completed == 3) {
      nextReply = static_cast<uint8_t>(latchedButtons >> 8);
    } else if (completed == 4) {
      ++psPollCount;
    }
  }

  psByteIndex = completed + 1;
  armPs1Byte(nextReply);
  if (psControllerSelected && psByteIndex < 5) pulseAck();
}

bool beginPs1Spi() {
  pinMode(DATA_DRIVE_PIN, OUTPUT);
  pinMode(ACK_DRIVE_PIN, OUTPUT);
  pinMode(CMD_SENSE_PIN, INPUT);
  pinMode(CLK_SENSE_PIN, INPUT);
  pinMode(ATT_SENSE_PIN, INPUT);
  releaseData();
  fastDrive(ACK_DRIVE_PIN, false);

  pinMatrixInAttach(CMD_SENSE_PIN, HSPID_IN_IDX, false);
  pinMatrixInAttach(CLK_SENSE_PIN, HSPICLK_IN_IDX, false);
  pinMatrixInAttach(ATT_SENSE_PIN, HSPICS0_IN_IDX, false);
  periph_module_enable(PERIPH_HSPI_MODULE);
  periph_module_reset(PERIPH_HSPI_MODULE);

  SPI2.clock.val = 0;
  SPI2.user.val = 0;
  SPI2.ctrl.val = 0;
  SPI2.slave.val = 0;
  SPI2.slave.wr_rd_buf_en = 1;
  SPI2.user.doutdin = 1;
  SPI2.slave.slave_mode = 1;
  SPI2.ctrl.wr_bit_order = 1;
  SPI2.ctrl.rd_bit_order = 1;
  SPI2.pin.ck_idle_edge = 0;
  SPI2.user.ck_i_edge = 1;
  SPI2.ctrl2.miso_delay_mode = 0;
  SPI2.ctrl2.miso_delay_num = 0;
  SPI2.ctrl2.mosi_delay_mode = 0;
  SPI2.ctrl2.mosi_delay_num = 0;
  SPI2.slv_wrbuf_dlen.bit_len = 7;
  SPI2.slv_rdbuf_dlen.bit_len = 7;
  SPI2.user.usr_miso = 1;
  SPI2.user.usr_mosi = 1;
  SPI2.data_buf[0] = 0x00;
  SPI2.slave.trans_inten = 1;
  SPI2.slave.trans_done = 0;

  const esp_err_t result = esp_intr_alloc(
      ETS_SPI2_INTR_SOURCE, ESP_INTR_FLAG_LEVEL1, onPs1SpiByte, nullptr,
      &psSpiInterrupt);
  if (result != ESP_OK) {
    psSpiInitError = result;
    return false;
  }

  psByteIndex = 0;
  psControllerSelected = false;
  SPI2.cmd.usr = 1;
  attachInterrupt(ATT_SENSE_PIN, onAttentionRising, RISING);
  psSpiReady = true;
  return true;
}

void handleRoot() {
  static const char page[] PROGMEM = R"HTML(
<!doctype html><html><head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>PlayBridge V3 Controller</title>
<style>body{font:18px system-ui;max-width:36rem;margin:2rem auto;padding:1rem}button{font-size:18px;margin:.25rem;padding:.75rem 1rem;touch-action:none}.held{color:white;background:#1769e0}.grid{display:grid;grid-template-columns:repeat(3,max-content);gap:.25rem}</style>
</head><body><h1>PlayBridge V3 Controller</h1>
<p>Controller input remains active while the serial bridge transfers data.</p>
<div><button data-pad="L2">L2</button><button data-pad="L1">L1</button><button data-pad="R1">R1</button><button data-pad="R2">R2</button></div>
<div class="grid"><span></span><button data-pad="UP">Up</button><span></span><button data-pad="LEFT">Left</button><button onclick="neutral()">Neutral</button><button data-pad="RIGHT">Right</button><span></span><button data-pad="DOWN">Down</button><span></span></div>
<div><button data-pad="SELECT">Select</button><button data-pad="START">Start</button></div>
<div><button data-pad="TRIANGLE">Triangle</button><button data-pad="CIRCLE">Circle</button><button data-pad="CROSS">Cross</button><button data-pad="SQUARE">Square</button></div>
<pre id="result">Ready</pre><script>
const states=new WeakMap(),minimumPressMs=80;
async function send(b,p){const r=await fetch('/button?name='+encodeURIComponent(b.dataset.pad)+'&pressed='+(p?'1':'0'));result.textContent=await r.text()}
function press(b){if(b.classList.contains('held'))return;b.classList.add('held');states.set(b,{at:performance.now(),request:send(b,true),releasing:false})}
async function release(b){const s=states.get(b);if(!s||s.releasing)return;s.releasing=true;try{await s.request}catch(_){}const wait=minimumPressMs-(performance.now()-s.at);if(wait>0)await new Promise(r=>setTimeout(r,wait));try{await send(b,false)}finally{b.classList.remove('held');states.delete(b)}}
async function neutral(){await fetch('/input?value=NEUTRAL');document.querySelectorAll('[data-pad]').forEach(b=>b.classList.remove('held'));states.clear?.()}
document.querySelectorAll('[data-pad]').forEach(b=>{b.onpointerdown=e=>{e.preventDefault();b.setPointerCapture(e.pointerId);press(b)};b.onpointerup=b.onpointercancel=e=>{e.preventDefault();release(b)};b.onlostpointercapture=()=>release(b)});
const result=document.getElementById('result');
setInterval(()=>fetch('/heartbeat',{cache:'no-store'}).catch(()=>{}),2000);
setInterval(async()=>{try{const r=await fetch('/bridge/status',{cache:'no-store'});result.textContent=await r.text()}catch(_){}},3000);
</script></body></html>)HTML";
  server.send(200, "text/html", page);
}

void handleInput() {
  if (!server.hasArg("value")) {
    server.send(400, "text/plain", "Missing input");
    return;
  }
  lastInput = server.arg("value");
  setPadInput(lastInput);
  server.send(200, "text/plain", "ESP32 received: " + lastInput);
}

void handleButton() {
  if (!server.hasArg("name") || !server.hasArg("pressed")) {
    server.send(400, "text/plain", "Missing button name or pressed state");
    return;
  }
  const String name = server.arg("name");
  const bool pressed = server.arg("pressed") == "1";
  if (!setButtonState(name, pressed)) {
    server.send(400, "text/plain", "Unknown PS1 button");
    return;
  }
  server.send(200, "text/plain",
              name + (pressed ? " pressed | " : " released | ") +
                  combinedPadReplyText());
}

void handleHeartbeat() {
  lastControllerHeartbeatMs = millis();
  server.send(204, "text/plain", "");
}

void handleBridgeStatus() {
  const Ps1GameBridge::Status status = Ps1GameBridge::snapshot();
  char json[768];
  snprintf(
      json, sizeof(json),
      "{\"firmware\":\"%s\",\"initialized\":%s,\"session\":%s,"
      "\"baud\":%lu,\"sessions\":%lu,\"tcp_rx\":%lu,\"uart_tx\":%lu,"
      "\"uart_rx\":%lu,\"tcp_tx\":%lu,\"uart_errors\":%lu,"
      "\"socket_errors\":%lu,\"aborted_bytes\":%lu,"
      "\"malformed_control\":%lu,\"duplicate_control\":%lu,"
      "\"pending_peak\":%lu,\"uart_rx_peak\":%lu,"
      "\"max_service_gap_us\":%lu,\"max_iteration_us\":%lu,"
      "\"pad_polls\":%lu,\"free_heap\":%lu}",
      kFirmwareVersion, status.initialized ? "true" : "false",
      status.sessionActive ? "true" : "false",
      static_cast<unsigned long>(status.baud),
      static_cast<unsigned long>(status.sessions),
      static_cast<unsigned long>(status.tcpReceived),
      static_cast<unsigned long>(status.uartWritten),
      static_cast<unsigned long>(status.uartReceived),
      static_cast<unsigned long>(status.tcpWritten),
      static_cast<unsigned long>(status.uartErrors),
      static_cast<unsigned long>(status.socketErrors),
      static_cast<unsigned long>(status.abortedBytes),
      static_cast<unsigned long>(status.malformedControl),
      static_cast<unsigned long>(status.duplicateControl),
      static_cast<unsigned long>(status.pendingPeak),
      static_cast<unsigned long>(status.uartRxPeak),
      static_cast<unsigned long>(status.maxServiceGapUs),
      static_cast<unsigned long>(status.maxIterationUs),
      static_cast<unsigned long>(psPollCount),
      static_cast<unsigned long>(ESP.getFreeHeap()));
  server.send(200, "application/json", json);
}

void bridgeTask(void *) {
  const bool ready = Ps1GameBridge::begin(16, 17);
  Serial.printf("UART2 bridge task core=%d ready=%s\n", xPortGetCoreID(),
                ready ? "YES" : "NO");
  for (;;) Ps1GameBridge::poll();
}

void printBridgeStatus() {
  const Ps1GameBridge::Status status = Ps1GameBridge::snapshot();
  Serial.printf(
      "Bridge ready=%s session=%s baud=%lu TCP RX/TX=%lu/%lu UART RX/TX=%lu/%lu errors=%lu gap=%lu us\n",
      status.initialized ? "YES" : "NO",
      status.sessionActive ? "YES" : "NO",
      static_cast<unsigned long>(status.baud),
      static_cast<unsigned long>(status.tcpReceived),
      static_cast<unsigned long>(status.tcpWritten),
      static_cast<unsigned long>(status.uartReceived),
      static_cast<unsigned long>(status.uartWritten),
      static_cast<unsigned long>(status.uartErrors + status.socketErrors),
      static_cast<unsigned long>(status.maxServiceGapUs));
}

void beginWifiConnection(const char *password = nullptr) {
  if (Ps1GameBridge::transferActive()) {
    Serial.println("BUSY: Wi-Fi changes are disabled during a transfer.");
    return;
  }
  if (password) WiFi.begin(LOCAL_WIFI_NAME, password);
  else WiFi.begin();
  wifiConnectPending = true;
  wifiConnectStartedMs = millis();
}

void serviceWifiConnection() {
  static uint32_t lastReconnectAttemptMs = 0;
  if (wifiConnectPending) {
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnectPending = false;
      Serial.printf("LOCAL WIFI CONNECTED | http://%s/\n",
                    WiFi.localIP().toString().c_str());
    } else if (millis() - wifiConnectStartedMs >= kWifiConnectTimeoutMs) {
      wifiConnectPending = false;
      Serial.println("Local Wi-Fi unavailable; fallback AP remains active.");
      lastReconnectAttemptMs = millis();
    }
  }

  if (!wifiConnectPending && WiFi.status() != WL_CONNECTED &&
      !Ps1GameBridge::transferActive() &&
      millis() - lastReconnectAttemptMs >= kWifiConnectTimeoutMs) {
    lastReconnectAttemptMs = millis();
    WiFi.reconnect();
    wifiConnectPending = true;
    wifiConnectStartedMs = millis();
  }
}

void handleSerialLine(const char *line) {
  if (awaitingWifiPassword) {
    awaitingWifiPassword = false;
    if (strcmp(line, "CANCEL") == 0) {
      Serial.println("Wi-Fi setup cancelled.");
    } else if (Ps1GameBridge::transferActive()) {
      Serial.println("BUSY: password change rejected during transfer.");
    } else {
      beginWifiConnection(line);
    }
    return;
  }

  if (strcmp(line, "SET_WIFI") == 0) {
    if (Ps1GameBridge::transferActive()) {
      Serial.println("BUSY: SET_WIFI rejected during transfer.");
      return;
    }
    awaitingWifiPassword = true;
    Serial.printf("Wi-Fi name: [%s]\nPASSWORD> ", LOCAL_WIFI_NAME);
  } else if (strcmp(line, "STATUS") == 0) {
    printStatus();
    printBridgeStatus();
    Serial.println(kFirmwareVersion);
  } else if (strcmp(line, "PAD_STATUS") == 0) {
    Serial.printf("PS1 digital reply: %s | Input: %s\n",
                  combinedPadReplyText().c_str(), lastInput.c_str());
  } else if (*line) {
    Serial.println("Unknown command. Use STATUS, PAD_STATUS, or SET_WIFI.");
  }
}

void serviceSerialInput() {
  int budget = 64;
  while (budget-- > 0 && Serial.available()) {
    const char value = static_cast<char>(Serial.read());
    if (value == '\r') continue;
    if (value == '\n') {
      serialLine[serialLineLength] = '\0';
      handleSerialLine(serialLine);
      memset(serialLine, 0, sizeof(serialLine));
      serialLineLength = 0;
    } else if (serialLineLength + 1 < sizeof(serialLine)) {
      serialLine[serialLineLength++] = value;
    }
  }
}

void enforceControllerExpiry() {
  if (padSnapshot() == 0xffff) return;
  if (millis() - lastControllerHeartbeatMs <= kControllerExpiryMs) return;
  setPadMask(0xffff);
  lastInput = "NEUTRAL (heartbeat expired)";
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(250);
  Serial.printf("%s\n", kFirmwareVersion);

  pinMode(ACK_DRIVE_PIN, OUTPUT);
  releaseBothOutputs();
  if (!beginPs1Spi()) Serial.println("PS1 controller responder unavailable.");

  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(false);
  // Reconnection is managed in loop() and is never started during a transfer.
  WiFi.setAutoReconnect(false);
  WiFi.softAP(AP_NAME, AP_PASSWORD);
  beginWifiConnection();

  server.on("/", handleRoot);
  server.on("/input", handleInput);
  server.on("/button", handleButton);
  server.on("/heartbeat", handleHeartbeat);
  server.on("/bridge/status", handleBridgeStatus);
  server.begin();

  lastControllerHeartbeatMs = millis();
  const BaseType_t created = xTaskCreatePinnedToCore(
      bridgeTask, "ps1-uart-bridge", kBridgeStackBytes, nullptr, 2,
      &bridgeTaskHandle, kBridgeCore);
  if (created != pdPASS) Serial.println("Failed to create UART bridge task.");

  Serial.printf("Controller task core=%d | AP http://%s/\n", xPortGetCoreID(),
                WiFi.softAPIP().toString().c_str());
}

void loop() {
  server.handleClient();
  serviceSerialInput();
  serviceWifiConnection();
  enforceControllerExpiry();
  delay(1);
}
