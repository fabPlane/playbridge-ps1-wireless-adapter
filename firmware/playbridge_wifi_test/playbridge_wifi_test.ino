#include <WiFi.h>
#include <WebServer.h>
#include <esp_rom_sys.h>
#include <soc/gpio_struct.h>

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
bool dataPulledLow = false;
bool ackPulledLow = false;

// PS1 digital-controller transaction state. The console sends and receives
// least-significant bit first. DATA and ACK are inverted by the NPN stages:
// GPIO HIGH pulls the PlayStation line low; GPIO LOW releases it.
volatile bool psTransactionActive = false;
volatile bool psControllerSelected = false;
volatile uint8_t psByteIndex = 0;
volatile uint8_t psBitIndex = 0;
volatile uint8_t psReceivedByte = 0;
volatile uint8_t psReply[5] = {0xFF, 0x41, 0x5A, 0xFF, 0xFF};
volatile uint32_t psPollCount = 0;
volatile uint32_t psAttFallingCount = 0;
volatile uint32_t psClockEdgeCount = 0;
volatile uint32_t psAddressMatchCount = 0;
volatile uint32_t psCommand42Count = 0;
volatile uint32_t psLastAttFallingCycle = 0;
volatile uint8_t psLastAddress = 0;
volatile uint8_t psLastCommand = 0;

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
  esp_rom_delay_us(7);
  fastDrive(ACK_DRIVE_PIN, true);
  esp_rom_delay_us(3);
  fastDrive(ACK_DRIVE_PIN, false);
}

void ARDUINO_ISR_ATTR onAttentionChange() {
  const uint32_t now = esp_cpu_get_cycle_count();
  const uint32_t sincePreviousFalling = now - psLastAttFallingCycle;
  psLastAttFallingCycle = now;

  // A real ATT start is separated from the previous poll by milliseconds.
  // Reject closely-spaced falling edges caused by breadboard/parallel-wire
  // crosstalk so they cannot repeatedly reset an in-progress byte.
  if (sincePreviousFalling < 24000U) return;  // 100 us at 240 MHz

  psAttFallingCount++;
  const uint16_t buttons = psButtons;
  psReply[3] = buttons & 0xFF;
  psReply[4] = buttons >> 8;
  psByteIndex = 0;
  psBitIndex = 0;
  psReceivedByte = 0;
  psControllerSelected = false;
  psTransactionActive = true;
  fastDrive(DATA_DRIVE_PIN, false);
  fastDrive(ACK_DRIVE_PIN, false);
}

void ARDUINO_ISR_ATTR onClockFalling() {
  psClockEdgeCount++;
  if (!psTransactionActive || fastReadInput(ATT_SENSE_PIN)) return;

  // PS1 transfers are LSB-first. Both transmitters change their data on the
  // falling CLK edge and the receivers sample on the following rising edge.
  // Handling only FALLING halves the interrupt rate and gives DATA almost the
  // whole low phase to settle before the console samples it.
  if (psByteIndex == 0 || !psControllerSelected || psByteIndex >= 5) {
    fastDrive(DATA_DRIVE_PIN, false);
  } else {
    const bool replyBitIsOne = (psReply[psByteIndex] >> psBitIndex) & 1U;
    fastDrive(DATA_DRIVE_PIN, !replyBitIsOne);
  }

  // By the time the GPIO interrupt runs, CMD for this bit has changed and is
  // stable. Sampling here also avoids a second interrupt on the rising edge.
  if (fastReadInput(CMD_SENSE_PIN)) psReceivedByte |= 1U << psBitIndex;
  psBitIndex++;
  if (psBitIndex < 8) return;

  const uint8_t completedByte = psByteIndex;
  const uint8_t received = psReceivedByte;
  psByteIndex++;
  psBitIndex = 0;
  psReceivedByte = 0;

  if (completedByte == 0) {
    psLastAddress = received;
    psControllerSelected = received == 0x01;
    if (psControllerSelected) psAddressMatchCount++;
  } else if (completedByte == 1 && received != 0x42) {
    psLastCommand = received;
    psControllerSelected = false;
  } else if (completedByte == 1) {
    psLastCommand = received;
    psCommand42Count++;
  }

  if (psControllerSelected && completedByte < 4) {
    pulseAck();
  }
  if (psControllerSelected && completedByte == 4) psPollCount++;
}

void setOpenCollectorDrive(uint8_t pin, bool pullLow) {
  // A HIGH GPIO drives the NPN base and pulls the collector low.
  // A LOW GPIO turns the transistor off and releases the collector.
  digitalWrite(pin, pullLow ? HIGH : LOW);
}

void releaseBothOutputs() {
  dataPulledLow = false;
  ackPulledLow = false;
  setOpenCollectorDrive(DATA_DRIVE_PIN, false);
  setOpenCollectorDrive(ACK_DRIVE_PIN, false);
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
  Serial.printf("ATT starts: %lu | CLK edges: %lu | Address 01: %lu | Command 42: %lu\n",
                (unsigned long)psAttFallingCount,
                (unsigned long)psClockEdgeCount,
                (unsigned long)psAddressMatchCount,
                (unsigned long)psCommand42Count);
  Serial.printf("Last address: %02X | Last command: %02X\n",
                psLastAddress, psLastCommand);
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
    async function setButton(button, pressed) {
      button.classList.toggle('held', pressed);
      const response = await fetch('/button?name=' + encodeURIComponent(button.dataset.pad) + '&pressed=' + (pressed ? '1' : '0'));
      document.getElementById('result').textContent = await response.text();
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
        setButton(button, true);
      });
      const release = event => {
        event.preventDefault();
        setButton(button, false);
      };
      button.addEventListener('pointerup', release);
      button.addEventListener('pointercancel', release);
      button.addEventListener('lostpointercapture', () => {
        if (button.classList.contains('held')) setButton(button, false);
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
    dataPulledLow = pullLow;
    setOpenCollectorDrive(DATA_DRIVE_PIN, pullLow);
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

  pinMode(DATA_DRIVE_PIN, OUTPUT);
  pinMode(ACK_DRIVE_PIN, OUTPUT);
  releaseBothOutputs();
  pinMode(CMD_SENSE_PIN, INPUT);
  pinMode(CLK_SENSE_PIN, INPUT);
  pinMode(ATT_SENSE_PIN, INPUT);
  attachInterrupt(ATT_SENSE_PIN, onAttentionChange, FALLING);
  attachInterrupt(CLK_SENSE_PIN, onClockFalling, FALLING);

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
