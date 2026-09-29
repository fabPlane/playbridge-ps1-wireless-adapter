# PlayBridge PS1 Wireless Adapter

PlayBridge is an ESP32-based wireless bridge that receives PC controller state
over Wi-Fi and presents it to an original PlayStation through the native PS1
controller protocol.

## Hardware versions

| Version | Design | Status |
| --- | --- | --- |
| `v1.0.0` | Direct PCB-mounted PS1 pin array | Frozen prototype checkpoint under [`versions/v1-pcb-mounted-pin-array/`](versions/v1-pcb-mounted-pin-array/) |
| `v2.0.0` | Modular donor-plug harness through board-mounted 9-contact connectors | Frozen release candidate under [`versions/v2-donor-plug-modular-adapter/`](versions/v2-donor-plug-modular-adapter/) |
| `v2.1.0` | V2.0 baseline plus status LEDs and UART debug access | Release candidate under [`versions/v2.1-status-leds-uart-debug/`](versions/v2.1-status-leds-uart-debug/) |
| `v3.0.0` | Combined PS1 controller and serial-harness evaluation | Frozen development checkpoint under [`versions/v3.0-ps1-serial-harness/`](versions/v3.0-ps1-serial-harness/) |
| `v4.0.0` | New two-layer controller/serial bridge with protected dual-source power | Digitally verified design under [`versions/v4.0-controller-serial-bridge/`](versions/v4.0-controller-serial-bridge/); physical qualification pending |

Each hardware version is a self-contained FabDesk project under `versions/`.
Open the version directory itself in FabDesk; the repository root is only the
project index plus shared firmware and documentation.

## V4.0.0 status

- Board: 68 × 60 mm, two copper layers.
- J1: PS1 controller harness with open-drain DATA and ACK stages.
- J2: PS1 serial harness on ESP32 UART2.
- Power: protected USB-C and PS1 inputs, TPS2116 source mux, AP63203 buck,
  and AP2112K 3.3 V regulation.
- Programming: CH340C with manual BOOT and RESET switches; automatic DTR/RTS
  behavior is present but is not required for acceptance.
- Indicators: always-on power LED and firmware-controlled status LED.
- Digital checks: ERC, DRC, schematic/PCB parity, BOM/CPL reference parity,
  fabrication archive integrity, and export manifest all pass.
- Manufacturing: bare-board Gerbers and in-house assembly BOM/CPL are present.
- Production remains on hold until the current-limited power, source-switching,
  programming, J1, and J2 bench tests are recorded.

See [`versions/v4.0-controller-serial-bridge/README.md`](versions/v4.0-controller-serial-bridge/README.md)
for the design summary and exact connector/GPIO maps.

## V3.0 and V4.0 comparison

| Area | V3.0 | V4.0 | Compatibility / action |
| --- | --- | --- | --- |
| MCU | ESP32-WROOM-32E-N4 | ESP32-WROOM-32E-N4 | Same module |
| J1 harness pins | 2 CMD, 3 CLK, 4 GND, 5 ATT, 6 DATA, 7 ACK, 8 PS1 power; 1/9 NC | Same signal allocation | Harness pin allocation retained |
| J2 harness pins | 1 ESP32 TX, 4 ESP32 RX, 7 GND; other signal pins NC | Same signal allocation | Harness pin allocation retained |
| J1/J2 connector | Hong Cheng HC-USB3.0-L168-ZP / C7501856 | Same connector | Physical connector and nine signal-pad centers retained; local footprint definitions differ |
| J1/J2 shells | Mechanically anchored and electrically isolated | J1 shells tied to pad 4/GND; J2 shells tied to pad 7/GND | Grounding policy changed |
| J1/J2 placement | J1 `(8.025, 50.825)` mm; J2 `(26.025, 50.825)` mm | J1 `(14.0, 48.5)` mm; J2 `(39.2, 48.5)` mm | Both face outward at the lower edge, but the PCB/enclosure layout is not drop-in compatible |
| Controller GPIOs | CMD 32, CLK 33, ATT 34, DATA drive 19, ACK drive 21 | CMD 19, CLK 32, ATT 21, DATA drive 33, ACK drive 27, ACK sense 34 | Controller firmware pin map must change |
| DATA/ACK stages | 2N7002 open-drain DATA and ACK drivers; no separate ACK input | Same drivers plus dedicated ACK sense on GPIO34 | Drive method retained; V4 firmware must use separate ACK drive/sense pins |
| J2 UART | GPIO17 TX / GPIO16 RX | GPIO17 TX / GPIO16 RX | Firmware-compatible |
| USB-C/programming | HRO TYPE-C-31-M-12, CH340C, manual BOOT/RESET, DTR/RTS transistor network | Same connector and programming functions | Manual method retained; V4 automatic behavior is not bench-qualified |
| Power conversion | AP63203 PS1 buck, AP2112K USB LDO, TPS2113A mux | Same buck/LDO, TPS2116 mux | Mux implementation changed; bench-test source switching and backfeed |
| Protection | USBLC6 USB ESD, SMBJ12A PS1 TVS, BAT54 clamps on CMD/CLK/ATT | SMAJ5.0A/SMAJ12A input TVS, TPD2EUSB30A arrays, dedicated ACK ESD | Protection architecture changed and requires prototype validation |
| LEDs | Red power LED; yellow-green status LED on GPIO4 | Green power LED; blue status LED on GPIO25 | Functions retained; status GPIO and colors changed |
| PCB | 56.295 × 62.000 mm, four layers, 1.20 mm thick | 68 × 60 mm, two layers, 1.60 mm thick | Dimensions and stackup changed |
| Validation | J1 baseline and J2 three-wire serial PoC have recorded bench evidence; digital/manufacturing gates pass | ERC/DRC/parity/order-package checks pass; physical bench plan is not yet executed | V4 is suitable for first-prototype ordering, not production approval |

## V2.0.0 historical status

![PlayBridge V2.0 donor-plug modular adapter board](versions/v2-donor-plug-modular-adapter/renders/v2-donor-plug-modular-adapter-angle.png)

_Board render created with [PCB Fiddle](https://pcbfiddle.com/)._

- Board: 56.295 × 62.000 mm, four copper layers, 1.20 mm thick.
- J1: 9-contact non-USB connector for a genuine PS1 donor-plug harness.
- J2: mechanically identical reserved connector; every contact is intentionally NC.
- J3: USB-C programming/power receptacle on the right edge, facing outward.
- J1 and J2: lower-edge mating mouths face outward and remain registered to the approved Molex footprints.
- Routing: complete; zero unconnected items.
- DRC: zero errors and zero warnings.
- ERC: zero errors and zero warnings.
- Schematic/PCB parity: zero findings.
- FabDesk release gate: PASS.
- PCBA data: 63 fitted references, 32 grouped BOM lines, and four DNP test pads.
- Controller-port bench firmware passed visible original-PS1 system-menu testing over Wi-Fi: all supported digital buttons worked, quick clicks were reliable, and the final check recorded 17,913 complete polls.

The design release is electrically and mechanically validated. Ordering still
requires a final live JLCPCB stock check and review of the uploaded BOM/CPL
rotation previews; see
[`versions/v2-donor-plug-modular-adapter/ordering/DO-NOT-ORDER.md`](versions/v2-donor-plug-modular-adapter/ordering/DO-NOT-ORDER.md).

## Repository layout

- `versions/v1-pcb-mounted-pin-array/` — complete frozen V1 FabDesk project, fit checks, historical renders, and provisional-contact studies.
- `versions/v2-donor-plug-modular-adapter/` — complete V2.0.0 FabDesk project, manufacturing package, reports, tools, references, and accepted renders.
- `versions/v2.1-status-leds-uart-debug/` — complete V2.1.0 FabDesk project, debug interface, manufacturing package, reports, tools, and accepted renders.
- `versions/v3.0-ps1-serial-harness/` — complete V3.0 controller/serial-harness development checkpoint and PoC evidence.
- `versions/v4.0-controller-serial-bridge/` — complete V4.0 FabDesk project, local libraries, validation evidence, renders, and manufacturing package.
- `firmware/playbridge_wifi_test/` — shared ESP32 Wi-Fi controller firmware and bench instructions.
- `docs/` — shared protocol and modular-interconnect diagrams.

## Opening a hardware project

In FabDesk, select the desired directory under `versions/`, not the repository
root. New hardware work should begin by copying the latest applicable version
to a new version directory so previous releases remain unchanged.

## Important connector warning

J1 and J2 use USB 3 Type-A connector hardware only as inexpensive keyed
9-contact interconnects. They carry native console signals and are **NOT USB**.
Never connect either port to a computer, charger, or ordinary USB peripheral.
