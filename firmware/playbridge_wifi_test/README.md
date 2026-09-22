# PlayBridge PS1 wireless bench firmware

This Arduino sketch turns an ESP32 into an experimental digital PlayStation controller. A browser sends button state to the ESP32 over Wi-Fi, and the ESP32 answers controller polls from an original PlayStation through the native controller-port protocol.

This is bench-validation firmware, not production firmware. The breadboard interface described below has been validated on an original PlayStation with visible output captured through OBS. All supported digital buttons were exercised from the browser UI and produced the expected console response. Validation across multiple consoles, games, PCB revisions, and long-duration sessions is still required.

## Current capabilities

- Digital-controller identity (`0x41`)
- D-pad, Select, Start, shoulder buttons, and four face buttons
- Browser controller served directly by the ESP32
- Ordered browser press/release requests with an 80 ms minimum press time for reliable quick clicks
- Local-network station mode plus a fallback access point
- Serial Wi-Fi setup and PS1 protocol diagnostics
- Byte-at-a-time SPI2 responder for complete five-byte PS1 polls
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

Open the ESP32 address in a browser. Press and hold a button to assert it; release the pointer to release it. Very quick clicks are automatically stretched to at least 80 ms, and the release request is sent only after the press request completes. The page provides:

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

- `STATUS` — Wi-Fi addresses, SPI state, completed-poll and transaction counters, and the last raw request
- `PAD_STATUS` — current five-byte controller reply and most recent browser input
- `SET_WIFI` — update the password for `LOCAL_WIFI_NAME`

A healthy digital poll uses console address `01`, command `42`, and controller reply:

```text
FF 41 5A <button-low-byte> <button-high-byte>
```

`PS1 completed polls` should continue increasing while the console is polling controller port 1. The decoded `01`/`42` counts and complete five-byte requests are the most useful health checks.

## Validated bench result

On 2026-09-22, the prototype completed a visible end-to-end test using controller port 1 on an original PlayStation and OBS video capture:

1. The PS1 continuously issued complete five-byte `01`/`42` controller requests.
2. The system menu remained stable without phantom or stuck input after reset.
3. Up, Down, Left, Right, Select, Start, L1, L2, R1, R2, Triangle, Circle, Cross, and Square were each tested from the browser UI.
4. Every press produced the expected active-low reply and visible menu action; every release restored `FF 41 5A FF FF`.
5. Quick clicks became reliable after ordered press/release requests and an 80 ms minimum press duration were added.
6. The final status check reported 17,913 completed polls and 89,655 SPI bytes.

The tested connection was `Browser -> Wi-Fi -> ESP32 -> breadboard signal-conditioning circuit -> PS1 controller port 1`.

This verifies the full path from a browser command to a visible response in the PS1 system menu.

## Implementation notes

- PS1 bytes are transferred least-significant bit first.
- SPI2 handles one eight-bit byte per transaction and is re-armed between bytes so ACK can be generated at the protocol boundary.
- DATA remains high-impedance until the responder sees controller address `0x01`; memory-card address `0x81` is ignored.
- ACK is asserted after each of the first four reply bytes following a 14 microsecond wait, then released after a 2 microsecond pulse.
- DATA and ACK logic is inverted by the external NPN stages: GPIO high pulls the PS1 line low, while GPIO low releases it.
- The SPI MISO timing is configured for the delay introduced by the external NPN stage and was validated across the full 40-bit poll.
- The implementation answers as a five-byte digital controller: `FF 41 5A buttons-low buttons-high`.

The byte-at-a-time SPI responder is adapted from the [BlueRetro PSX SPI implementation](https://github.com/darthcloud/BlueRetro), which is licensed under Apache-2.0. The source file retains the corresponding attribution notice.

## Known limitations

- The direct-register SPI implementation is intended for the validated classic ESP32 and Arduino core configuration.
- Breadboard jumpers and long parallel wires can introduce crosstalk and intermittent contacts.
- The fallback AP uses a development password and the webpage is unauthenticated.
- The firmware currently contains a build-time local SSID constant.
- No gameplay test, analog-mode test, vibration test, multi-console test, or long-duration reliability test has been completed.
- Do not treat this bench circuit as a production electrical or mechanical release.
