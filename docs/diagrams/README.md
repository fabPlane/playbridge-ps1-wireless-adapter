# PlayBridge modular interconnect diagrams

These diagrams document the proposed two-connector modular architecture. The
USB 3 Type-A shells are used only as inexpensive **nine-contact mechanical
connectors**. The signals are native PlayStation/ESP32 signals, not USB data,
so the finished hardware must be labeled to prevent connection to a USB host.

## Rendered diagrams

- [System architecture](./ps1-modular-interconnect.svg)
- [Front-port harness](./ps1-front-harness.svg)
- [Proposed contact allocation](./ps1-contact-allocation.svg)
- [Photorealistic product concept](../images/concepts/playbridge-modular-harness-concept.png)

PNG exports are stored beside the SVG files for quick previews. The editable
Mermaid sources are the matching `.mmd` files.

## Architecture summary

The main ESP32 PCB has two USB 3 female receptacles:

1. `J1 FRONT BUS` connects to a passive harness for two controller plugs and,
   optionally, two memory-card edge adapters.
2. `J2 SERIAL` connects to a passive rear-serial-port harness.

The front harness shares `DATA`, `CMD`, `CLK`, `ACK`, and `GND` between the two
PlayStation ports. Separate `ATT1` and `ATT2` lines select the two ports. This
mirrors the PlayStation bus topology; it is not a USB hub.

The proposed contact assignments are a design reference, not a finalized
pinout. Confirm voltage rails, ESP32 protection, the PS1 serial-port electrical
interface, connector orientation, and continuity on actual donor cables before
fabrication.

The product concept image is illustrative rather than dimensionally accurate.
Connector geometry, pin count, cable construction, enclosure clearances, and
the memory-card edge adapters must be validated against production parts.
