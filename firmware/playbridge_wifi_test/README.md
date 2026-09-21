# PlayBridge PS1 wireless bench firmware

This Arduino sketch turns an ESP32 into an experimental digital PlayStation controller. A browser sends button state to the ESP32 over Wi-Fi, and the ESP32 answers controller polls from an original PlayStation through the native controller-port protocol.

This is bench-validation firmware, not production firmware. It has been exercised with the breadboard interface described below and has completed live PS1 controller polls, but it still depends on timing-sensitive GPIO interrupt code and has not yet been validated across multiple consoles, games, PCB revisions, or long-duration sessions.

## Current capabilities

- Digital-controller identity (`0x41`)
- D-pad, Select, Start, shoulder buttons, and four face buttons
- Browser controller served directly by the ESP32
- Local-network station mode plus a fallback access point
- Serial Wi-Fi setup and PS1 protocol diagnostics
- Open-collector DATA and ACK outputs through external NPN stages
- Protected CMD, ATT, and CLK inputs

Analog sticks, vibration, pressure-sensitive buttons, authentication, and encrypted web control are not implemented.

## Safety and signal rules

The controller cable carries native PlayStation signals. It is not USB, even when a USB-style connector is used as a modular harness.

- Never connect the native-signal harness to a computer, charger, USB hub, or other USB device.
- Power the bench ESP32 from its normal USB development connection.
- Connect ESP32 ground to PS1 controller-port pin 4.
- Leave PS1 pin 3 (`~7.5 V`) isolated from the ESP32 and breadboard power rails.
- Leave PS1 pin 5 (`~3.5 V`) isolated from ESP32 `3V3` and USB power.
- Leave PS1 pin 8 disconnected.
- Turn off the PS1 before inserting, removing, or continuity-testing controller wiring.

## Bench pin mapping

| PS1 pin | Wire used in the validated harness | Signal | Bench connection |
| ---: | --- | --- | --- |
| 1 | White | DATA | NPN open-collector stage driven by GPIO19 |
| 2 | Black | CMD | 1 kΩ protected input to GPIO32 |
| 3 | Brown | Motor supply, approximately 7.5 V | Isolated |
| 4 | Yellow | Ground | ESP32 and breadboard ground |
| 5 | Orange | Console supply, approximately 3.5 V | Isolated |
| 6 | Red | ATT | 1 kΩ protected input to GPIO34 |
| 7 | Grey | CLK | 1 kΩ protected input to GPIO33 |
| 8 | Purple | Unused | Disconnected; the extension may omit this contact |
| 9 | Blue | ACK | NPN open-collector stage driven by GPIO21 |

The bench input stages use a 1 kΩ series resistor and BAT85 clamps to the ESP32 3.3 V and ground rails. DATA and ACK use external NPN transistors so the ESP32 only pulls the console lines low. A `104` (100 nF) capacitor decouples the breadboard 3.3 V and ground rails.

The current GPIO definitions are:

| Function | GPIO |
| --- | ---: |
| DATA drive | 19 |
| ACK drive | 21 |
| CMD sense | 32 |
| CLK sense | 33 |
| ATT sense | 34 |

## Connector orientation

Looking directly at the extension cable's male contact face with the key/top upward:

```text
1 2 3 | 4 5 6 | 7 8 9
```

Looking into its female socket with the key/top upward:

```text
9 8 7 | 6 5 4 | 3 2 1
```

Do not rely on wire colors alone. Confirm every harness with continuity mode while the PS1 and ESP32 are unpowered.

## Build requirements

The validated build used:

- Arduino CLI
- Espressif ESP32 Arduino core 3.3.11
- Board FQBN `esp32:esp32:node32s`
- A classic ESP32 NODE32LITE-style development board

Compile from the repository root:

```sh
arduino-cli compile --fqbn esp32:esp32:node32s firmware/playbridge_wifi_test
```

Upload, replacing the port with the ESP32 serial device on the development computer:

```sh
arduino-cli upload \
  --port /dev/cu.usbserial-DEVICE \
  --fqbn esp32:esp32:node32s \
  firmware/playbridge_wifi_test
```

## Wi-Fi setup

The sketch always starts a fallback access point:

- SSID: `PlayBridge-Test`
- Password: `playbridge`
- Controller page: `http://192.168.4.1/`

It also attempts to reconnect using credentials previously stored by the ESP32 Wi-Fi stack. Before building for another network, change `LOCAL_WIFI_NAME` near the top of `playbridge_wifi_test.ino`.

To save or refresh the password for that SSID:

1. Open the ESP32 serial monitor at 115200 baud.
2. Enter `SET_WIFI`.
3. Enter the Wi-Fi password when prompted. The sketch does not echo it.
4. After connection, use the local IP address printed by the ESP32.

The web server uses plain HTTP and has no authentication. Use it only on a trusted test network.

## Browser control

Open the ESP32 address in a browser. Press and hold a button to assert it; release the pointer to release it. The page provides:

- Up, Down, Left, and Right
- Select and Start
- L1, L2, R1, and R2
- Triangle, Circle, Cross, and Square
- Neutral, which releases all buttons

PS1 button bits are active-low. A neutral digital reply is:

```text
FF 41 5A FF FF
```

For example, holding Right produces:

```text
FF 41 5A DF FF
```

## Serial diagnostics

Open the serial monitor at 115200 baud and use:

- `STATUS` — Wi-Fi addresses, completed-poll count, edge counters, and last decoded address/command
- `PAD_STATUS` — current five-byte controller reply and most recent browser input
- `SET_WIFI` — update the password for `LOCAL_WIFI_NAME`

A healthy digital poll uses console address `01`, command `42`, and controller reply:

```text
FF 41 5A <button-low-byte> <button-high-byte>
```

`PS1 completed polls` should continue increasing while the console is polling controller port 1. Breadboard wiring can create extra ATT or CLK edges, so completed polls and decoded `01`/`42` counts are more meaningful than raw edge counts alone.

## Validated bench result

On 2026-09-21, the prototype completed a headless end-to-end test:

1. The PS1 continuously issued valid `01`/`42` controller requests.
2. Completed polls increased from 7,380 to 8,153 while Right was held.
3. A PC sent Right over Wi-Fi and the reply changed to `FF 41 5A DF FF`.
4. Releasing Right restored `FF 41 5A FF FF`.
5. Completed polls continued increasing to 8,703 after release.

This verifies the path `PC browser -> Wi-Fi -> ESP32 -> controller harness -> PS1 protocol`. A video-output test is still required to confirm visible behavior in actual menus and games.

## Implementation notes

- PS1 bytes are transferred least-significant bit first.
- CLK and ATT are handled on falling-edge GPIO interrupts.
- DATA is prepared during the CLK low phase for sampling on the next rising edge.
- DATA and ACK logic is inverted by the external NPN stages: GPIO high pulls the PS1 line low, while GPIO low releases it.
- A short ATT falling-edge filter reduces breadboard crosstalk resets.
- The implementation answers as a five-byte digital controller: `FF 41 5A buttons-low buttons-high`.

## Known limitations

- This timing-sensitive interrupt implementation is intended for the validated classic ESP32 configuration.
- The ATT filter assumes a 240 MHz ESP32 clock.
- Breadboard jumpers and long parallel wires can introduce crosstalk and intermittent contacts.
- The fallback AP uses a development password and the webpage is unauthenticated.
- The firmware currently contains a build-time local SSID constant.
- No game-screen test, analog-mode test, vibration test, or long-duration reliability test has been completed.
- Do not treat this bench circuit as a production electrical or mechanical release.
