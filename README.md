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
| J1/J2 | Hong Cheng C7501856; same signal pinout | Same | Harness-compatible |
| Shells | Isolated | J1→GND pad 4; J2→GND pad 7 | Grounding changed |
| Placement | J1 `(8.025,50.825)`; J2 `(26.025,50.825)` mm | J1 `(14,48.5)`; J2 `(39.2,48.5)` mm | Not mechanically drop-in |
| Controller GPIO | CMD32, CLK33, ATT34, DATA19, ACK21 | CMD19, CLK32, ATT21, DATA33, ACK27/sense34 | Firmware change |
| DATA/ACK | 2N7002 open-drain; no ACK sense | Same drivers plus ACK sense | Firmware change |
| J2 UART | TX17 / RX16 | TX17 / RX16 | Compatible |
| Power | AP63203 + AP2112K + TPS2113A | AP63203 + AP2112K + TPS2116 | Mux changed |
| Protection | USBLC6, SMBJ12A, BAT54 clamps | SMAJ TVS, TPD2EUSB30A, ACK ESD | Prototype-test required |
| LEDs | Red power; yellow-green status GPIO4 | Green power; blue status GPIO25 | GPIO/color change |
| Programming | CH340C; manual BOOT/RESET; DTR/RTS | Same | Auto mode unqualified |
| PCB | 56.295 × 62 mm; 4-layer; 1.2 mm | 68 × 60 mm; 2-layer; 1.6 mm | Size/stackup changed |
| Validation | J1/J2 PoC tested | Digital checks pass; bench pending | First prototype only |

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
