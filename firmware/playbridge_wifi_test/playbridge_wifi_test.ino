#include <WiFi.h>
#include <WebServer.h>
#include <esp_intr_alloc.h>
#include <esp_private/periph_ctrl.h>
#include <esp_rom_sys.h>
#include <soc/gpio_sig_map.h>
#include <soc/gpio_struct.h>
#include <soc/interrupts.h>
#include <soc/periph_defs.h>
#include <soc/spi_struct.h>
#include "esp32-hal-matrix.h"

const char *AP_NAME = "PlayBridge-Test";
const char *AP_PASSWORD = "playbridge";
const char *LOCAL_WIFI_NAME = "TensorfleetNet";

const uint8_t DATA_DRIVE_PIN = 19;
const uint8_t ACK_DRIVE_PIN = 21;
const uint8_t CMD_SENSE_PIN = 32;
const uint8_t CLK_SENSE_PIN = 33;
const uint8_t ATT_SENSE_PIN = 34;

WebServer server(80);
String lastInput = "NEUTRAL";
// PS1 digital-pad buttons are active-low: 0 = pressed, 1 = released.
// Byte 0: Select, L3, R3, Start, Up, Right, Down, Left.
// Byte 1: L2, R2, L1, R1, Triangle, Circle, Cross, Square.
volatile uint16_t psButtons = 0xFFFF;
uint8_t wifiSetupStep = 0;
bool ackPulledLow = false;

// PS1 byte engine adapted from BlueRetro's Apache-2.0 PSX SPI responder.
// It uses SPI2 one byte at a time instead of treating the entire controller
// packet as one transaction. This is important because a PS1 expects an ACK
// pulse between bytes. Our external NPN stages invert DATA and ACK, so reply
// bytes are complemented and GPIO HIGH means "pull the PS1 line low".
// Source: https://github.com/darthcloud/BlueRetro/blob/master/main/wired/ps_spi.c
volatile bool psSpiReady = false;
volatile bool psControllerSelected = false;
volatile uint8_t psByteIndex = 0;
volatile uint8_t psLastAddress = 0;
volatile uint8_t psLastCommand = 0;
volatile uint8_t psLastRx[5] = {0};
volatile uint32_t psPollCount = 0;
volatile uint32_t psAttFallingCount = 0;
volatile uint32_t psAddressMatchCount = 0;
volatile uint32_t psCommand42Count = 0;
volatile uint32_t psSpiByteCount = 0;
volatile int32_t psSpiInitError = 0;
intr_handle_t psSpiInterrupt = nullptr;

static inline void ARDUINO_ISR_ATTR fastDrive(uint8_t pin, bool pullLow) {
  const uint32_t mask = 1UL << pin;
  if (pullLow) GPIO.out_w1ts = mask;
  else GPIO.out_w1tc = mask;
}

static inline bool ARDUINO_ISR_ATTR fastReadInput(uint8_t pin) {
  if (pin < 32) return (GPIO.in >> pin) & 1U;
  return (GPIO.in1.val >> (pin - 32)) & 1U;
}

static inline void ARDUINO_ISR_ATTR pulseAck() {
  // PSX-SPX documents that ACKs in the first 2-3 us after the last clock are
  // ignored. BlueRetro waits 14 us and then holds ACK low for 2 us.
  fastDrive(ACK_DRIVE_PIN, true);
  esp_rom_delay_us(2);
  fastDrive(ACK_DRIVE_PIN, false);
}

static inline void ARDUINO_ISR_ATTR releaseData() {
  pinMatrixOutDetach(DATA_DRIVE_PIN, false, false);
  fastDrive(DATA_DRIVE_PIN, false);
}

static inline void ARDUINO_ISR_ATTR armPs1Byte(uint8_t busByte) {
  // The transistor inverts the ESP32 signal before it reaches PS1 DATA.
  SPI2.data_buf[0] = static_cast<uint8_t>(~busByte);
  SPI2.slave.sync_reset = 1;
  SPI2.slave.trans_done = 0;
  SPI2.cmd.usr = 1;
}

void ARDUINO_ISR_ATTR onAttentionRising() {
  psControllerSelected = false;
  psByteIndex = 0;
  releaseData();
  fastDrive(ACK_DRIVE_PIN, false);
  armPs1Byte(0xFF);
}

void ARDUINO_ISR_ATTR onPs1SpiByte(void *) {
  // Match the proven PSX timing: prepare the next byte before ACK, 14 us
  // after the completed byte. All code/data touched here is IRAM/DRAM safe.
  esp_rom_delay_us(14);

  const uint8_t completed = psByteIndex;
  const uint8_t received = static_cast<uint8_t>(SPI2.data_buf[0]);
  psSpiByteCount++;
  if (completed < sizeof(psLastRx)) psLastRx[completed] = received;

  uint8_t nextReply = 0xFF;
  if (completed == 0) {
    psAttFallingCount++;
    psLastAddress = received;
    psControllerSelected = received == 0x01;
    if (psControllerSelected) {
      psAddressMatchCount++;
      // DATA must remain high-impedance during the address byte. Only connect
      // SPI MISO after controller address 01h has been recognized.
      pinMatrixOutAttach(DATA_DRIVE_PIN, HSPIQ_OUT_IDX, false, false);
      nextReply = 0x41;
    }
  } else if (psControllerSelected) {
    if (completed == 1) {
      psLastCommand = received;
      if (received == 0x42) psCommand42Count++;
      nextReply = 0x5A;
    } else if (completed == 2) {
      nextReply = static_cast<uint8_t>(psButtons & 0xFF);
    } else if (completed == 3) {
      nextReply = static_cast<uint8_t>(psButtons >> 8);
    } else if (completed == 4) {
      psPollCount++;
    }
  }

  psByteIndex = completed + 1;
  armPs1Byte(nextReply);

  // A digital controller sends five bytes total and ACKs the first four.
  if (psControllerSelected && psByteIndex < 5) pulseAck();
}

void setOpenCollectorDrive(uint8_t pin, bool pullLow) {
  // A HIGH GPIO drives the NPN base and pulls the collector low.
  // A LOW GPIO turns the transistor off and releases the collector.
  digitalWrite(pin, pullLow ? HIGH : LOW);
}

void releaseBothOutputs() {
  ackPulledLow = false;
  setOpenCollectorDrive(ACK_DRIVE_PIN, false);
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
  // BlueRetro's proven original-PSX byte engine, with the output launch edge
  // matched to this board's external inverting NPN stage. Our earlier mode-2
  // hardware run completed all 40 clocks; delay mode 0 reproduces that MISO
  // launch edge while retaining BlueRetro's byte-at-a-time ACK architecture.
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
  SPI2.data_buf[0] = 0x00;  // inverted FF while DATA is disconnected
  SPI2.slave.trans_inten = 1;
  SPI2.slave.trans_done = 0;

  const esp_err_t result = esp_intr_alloc(
      ETS_SPI2_INTR_SOURCE,
      // Arduino-ESP32's prebuilt IDF rejects ESP_INTR_FLAG_IRAM for this
      // peripheral (ESP_ERR_INVALID_ARG). The handler itself remains in IRAM;
      // allocate the source with the supported level-1 flag.
      ESP_INTR_FLAG_LEVEL1,
      onPs1SpiByte, nullptr, &psSpiInterrupt);
  if (result != ESP_OK) {
    psSpiInitError = result;
    Serial.printf("PS1 SPI interrupt allocation failed: %d\n", result);
    return false;
  }

  psByteIndex = 0;
  psControllerSelected = false;
  SPI2.cmd.usr = 1;
  attachInterrupt(ATT_SENSE_PIN, onAttentionRising, RISING);
  psSpiReady = true;
  return true;
}

void printCommandPrompt() {
  Serial.println();
  Serial.println("Ready. Type SET_WIFI and press Return.");
  Serial.println("You can also type STATUS or PAD_STATUS and press Return.");
  Serial.print("> ");
}

void setPadInput(const String &input) {
  psButtons = 0xFFFF;

  if (input == "UP") {
    psButtons &= ~(1U << 4);
  } else if (input == "RIGHT") {
    psButtons &= ~(1U << 5);
  } else if (input == "DOWN") {
    psButtons &= ~(1U << 6);
  } else if (input == "LEFT") {
    psButtons &= ~(1U << 7);
  } else if (input == "CROSS") {
    psButtons &= ~(1U << 14);
  }
}

int8_t buttonBit(const String &name) {
  if (name == "SELECT") return 0;
  if (name == "START") return 3;
  if (name == "UP") return 4;
  if (name == "RIGHT") return 5;
  if (name == "DOWN") return 6;
  if (name == "LEFT") return 7;
  if (name == "L2") return 8;
  if (name == "R2") return 9;
  if (name == "L1") return 10;
  if (name == "R1") return 11;
  if (name == "TRIANGLE") return 12;
  if (name == "CIRCLE") return 13;
  if (name == "CROSS") return 14;
  if (name == "SQUARE") return 15;
  return -1;
}

bool setButtonState(const String &name, bool pressed) {
  const int8_t bit = buttonBit(name);
  if (bit < 0) return false;

  if (pressed) {
    psButtons &= ~(1U << bit);
    lastInput = name;
  } else {
    psButtons |= 1U << bit;
    lastInput = "RELEASE " + name;
  }
  return true;
}

String padReplyText() {
  char reply[15];
  snprintf(reply, sizeof(reply), "FF 41 5A %02X %02X",
           psButtons & 0xFF,
           psButtons >> 8);
  return String(reply);
}

void printPadStatus() {
  Serial.printf("PS1 digital reply: %s | Input: %s\n",
                padReplyText().c_str(),
                lastInput.c_str());
}

void printStatus() {
  Serial.printf("Fallback network: %s | http://%s/ | Last input: %s\n",
                AP_NAME,
                WiFi.softAPIP().toString().c_str(),
                lastInput.c_str());
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("Local Wi-Fi connected | http://%s/\n",
                  WiFi.localIP().toString().c_str());
  } else {
    Serial.println("Local Wi-Fi: not connected");
  }
  Serial.printf("PS1 completed polls: %lu\n", (unsigned long)psPollCount);
  Serial.println("Responder: byte-at-a-time SPI2 / inter-byte ACK");
  Serial.printf("SPI ready: %s | Init error: %ld | Controller selected: %s | Byte index: %u\n",
                psSpiReady ? "YES" : "NO",
                (long)psSpiInitError,
                psControllerSelected ? "YES" : "NO",
                psByteIndex);
  Serial.printf("SPI bytes: %lu | ATT transactions: %lu | ATT now: %u | CLK now: %u | CMD now: %u\n",
                (unsigned long)psSpiByteCount,
                (unsigned long)psAttFallingCount,
                digitalRead(ATT_SENSE_PIN), digitalRead(CLK_SENSE_PIN),
                digitalRead(CMD_SENSE_PIN));
  Serial.printf("Address 01: %lu | Command 42: %lu\n",
                (unsigned long)psAddressMatchCount,
                (unsigned long)psCommand42Count);
  Serial.printf("Last address: %02X | Last command: %02X\n",
                psLastAddress, psLastCommand);
  Serial.printf("Last raw receive: %02X %02X %02X %02X %02X\n",
                psLastRx[0], psLastRx[1], psLastRx[2], psLastRx[3],
                psLastRx[4]);
}

void connectToLocalWifi(const String &ssid, const String &password) {
  Serial.printf("Connecting to local Wi-Fi '%s'", ssid.c_str());
  WiFi.begin(ssid.c_str(), password.c_str());

  const unsigned long started = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - started < 15000) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("LOCAL WIFI CONNECTED");
    Serial.printf("While your Mac stays on its normal Wi-Fi, open: http://%s/\n",
                  WiFi.localIP().toString().c_str());
  } else {
    Serial.println("LOCAL WIFI FAILED; the PlayBridge-Test access point is still available.");
  }
  printCommandPrompt();
}

void connectUsingSavedWifi() {
  Serial.printf("Reconnecting to saved Wi-Fi '%s'", LOCAL_WIFI_NAME);
  WiFi.setAutoReconnect(true);
  WiFi.begin();

  const unsigned long started = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - started < 12000) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("LOCAL WIFI CONNECTED | http://%s/\n",
                  WiFi.localIP().toString().c_str());
  } else {
    Serial.println("Saved Wi-Fi connection unavailable; use SET_WIFI once to refresh it.");
  }
}

void handleSerialSetup() {
  if (!Serial.available()) return;

  String line = Serial.readStringUntil('\n');
  line.trim();

  if (wifiSetupStep == 0 && line == "SET_WIFI") {
    wifiSetupStep = 1;
    Serial.printf("Wi-Fi name is fixed as: [%s]\n", LOCAL_WIFI_NAME);
    Serial.println("Enter Wi-Fi password, then press Return (the ESP32 will not print it):");
    Serial.print("PASSWORD> ");
  } else if (wifiSetupStep == 0 && line == "STATUS") {
    printStatus();
    printCommandPrompt();
  } else if (wifiSetupStep == 0 && line == "PAD_STATUS") {
    printPadStatus();
    printCommandPrompt();
  } else if (wifiSetupStep == 1) {
    if (line == "CANCEL") {
      wifiSetupStep = 0;
      Serial.println("Wi-Fi setup cancelled.");
      printCommandPrompt();
      return;
    }
    String password = line;
    wifiSetupStep = 0;
    connectToLocalWifi(LOCAL_WIFI_NAME, password);
    password = "";
  } else {
    Serial.println("Unknown command.");
    printCommandPrompt();
  }
}

void handleRoot() {
  const char page[] PROGMEM = R"HTML(
<!doctype html>
<html>
<head>
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>PlayBridge Controller</title>
  <style>
    body { font: 18px system-ui; max-width: 36rem; margin: 3rem auto; padding: 1rem; }
    button { font-size: 18px; margin: .25rem; padding: .75rem 1rem; touch-action: none; }
    button.held { color: white; background: #1769e0; }
    .controller { display: grid; grid-template-columns: repeat(3, max-content); gap: .25rem; align-items: center; }
    .shoulders, .system, .faces { margin-top: 1rem; }
    #result { margin-top: 1rem; font-weight: bold; }
  </style>
</head>
<body>
  <h1>PlayBridge Controller</h1>
  <p>Hold a button to press it; release it to release the PS1 button.</p>
  <div class="shoulders">
    <button data-pad="L2">L2</button><button data-pad="L1">L1</button>
    <button data-pad="R1">R1</button><button data-pad="R2">R2</button>
  </div>
  <div class="controller">
    <span></span><button data-pad="UP">Up</button><span></span>
    <button data-pad="LEFT">Left</button><button onclick="releaseAll()">Neutral</button><button data-pad="RIGHT">Right</button>
    <span></span><button data-pad="DOWN">Down</button><span></span>
  </div>
  <div class="system">
    <button data-pad="SELECT">Select</button><button data-pad="START">Start</button>
  </div>
  <div class="faces">
    <button data-pad="TRIANGLE">Triangle</button><button data-pad="CIRCLE">Circle</button>
    <button data-pad="CROSS">Cross</button><button data-pad="SQUARE">Square</button>
  </div>
  <div id="result">Ready</div>
  <script>
    const buttonState = new WeakMap();
    const minimumPressMs = 80;

    async function sendButton(button, pressed) {
      const response = await fetch('/button?name=' + encodeURIComponent(button.dataset.pad) + '&pressed=' + (pressed ? '1' : '0'));
      document.getElementById('result').textContent = await response.text();
    }

    function pressButton(button) {
      if (button.classList.contains('held')) return;
      button.classList.add('held');
      buttonState.set(button, {
        pressedAt: performance.now(),
        pressRequest: sendButton(button, true),
        releasing: false
      });
    }

    async function releaseButton(button) {
      const state = buttonState.get(button);
      if (!state || state.releasing) return;
      state.releasing = true;
      // Preserve request order, then keep even a very quick click pressed long
      // enough to span multiple PS1 controller polls.
      try { await state.pressRequest; } catch (_) {}
      const remaining = minimumPressMs - (performance.now() - state.pressedAt);
      if (remaining > 0) await new Promise(resolve => setTimeout(resolve, remaining));
      try { await sendButton(button, false); } finally {
        button.classList.remove('held');
        buttonState.delete(button);
      }
    }

    async function releaseAll() {
      const response = await fetch('/input?value=NEUTRAL');
      document.querySelectorAll('[data-pad]').forEach(button => button.classList.remove('held'));
      document.getElementById('result').textContent = await response.text();
    }
    document.querySelectorAll('[data-pad]').forEach(button => {
      button.addEventListener('pointerdown', event => {
        event.preventDefault();
        button.setPointerCapture(event.pointerId);
        pressButton(button);
      });
      const release = event => {
        event.preventDefault();
        releaseButton(button);
      };
      button.addEventListener('pointerup', release);
      button.addEventListener('pointercancel', release);
      button.addEventListener('lostpointercapture', () => {
        releaseButton(button);
      });
    });
  </script>
</body>
</html>
)HTML";
  server.send(200, "text/html", page);
}

void handleInput() {
  if (!server.hasArg("value")) {
    server.send(400, "text/plain", "Missing input");
    return;
  }

  lastInput = server.arg("value");
  setPadInput(lastInput);
  Serial.printf("Received over Wi-Fi: %s\n", lastInput.c_str());
  printPadStatus();
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

  printPadStatus();
  server.send(200, "text/plain",
              name + (pressed ? " pressed | " : " released | ") + padReplyText());
}

void handleSignal() {
  if (!server.hasArg("line") || !server.hasArg("action")) {
    server.send(400, "text/plain", "Missing line or action");
    return;
  }

  const String line = server.arg("line");
  const String action = server.arg("action");
  const bool pullLow = action == "PULL";

  if (action != "PULL" && action != "RELEASE") {
    server.send(400, "text/plain", "Unknown action");
    return;
  }

  if (line == "DATA") {
    server.send(409, "text/plain", "DATA is owned by the hardware SPI responder");
    return;
  } else if (line == "ACK") {
    ackPulledLow = pullLow;
    setOpenCollectorDrive(ACK_DRIVE_PIN, pullLow);
  } else if (line == "ALL" && !pullLow) {
    releaseBothOutputs();
  } else {
    server.send(400, "text/plain", "Unknown line");
    return;
  }

  const String message = line + ": " + (pullLow ? "PULLED LOW" : "RELEASED");
  Serial.println(message);
  server.send(200, "text/plain", message);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(ACK_DRIVE_PIN, OUTPUT);
  releaseBothOutputs();
  if (!beginPs1Spi()) {
    Serial.println("PS1 hardware responder unavailable.");
  }

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_NAME, AP_PASSWORD);
  connectUsingSavedWifi();

  server.on("/", handleRoot);
  server.on("/input", handleInput);
  server.on("/button", handleButton);
  server.begin();

  Serial.println("PlayBridge PS1 digital controller is ready.");
  Serial.printf("Network: %s\n", AP_NAME);
  Serial.printf("Open: http://%s/\n", WiFi.softAPIP().toString().c_str());
  printCommandPrompt();
}

void loop() {
  server.handleClient();
  handleSerialSetup();
}
