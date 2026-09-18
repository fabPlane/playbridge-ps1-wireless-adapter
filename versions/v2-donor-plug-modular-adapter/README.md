# V2 — donor-plug modular adapter

V2 is the planned replacement for the PCB-mounted PlayStation pin array.

The main PlayBridge board will expose removable multi-contact connectors. A console-specific adapter will use a genuine donor controller cable and molded PlayStation male plug soldered to a matching breakout board. Swapping the adapter cable will make it possible to support other console systems without manufacturing their proprietary plug contacts directly on the main PCB.

## Initial plan

- Keep the ESP32, signal protection and protocol-generation circuitry on the main board.
- Replace the console-facing PCB pin array with a removable cable interface.
- Use the genuine molded PlayStation donor plug for console alignment, contact geometry and insertion loads.
- Map every donor wire by continuity testing; never rely on wire color.
- Treat the observed missing pin 8 as a donor-plug property that must be verified electrically.
- Define connector pin allocation, power/backfeed protection, shield policy and cable requirements before designing the V2 PCB.
- Add other console adapters only after the PS1 harness is validated.

## Safety warning

The concept currently uses USB 3 Type-A connector hardware because it provides enough inexpensive contacts and readily available breakout boards. The signals would be native console signals—not USB. Any implementation must be clearly marked **NOT USB**, protected against accidental connection to computers/chargers, and reviewed before hardware is powered.

## References

- [`concepts/`](concepts/) contains the supplied system architecture, breakout-board example and illustrative product layout.
- [`references/ps1-donor-plug/`](references/ps1-donor-plug/) contains photographs of the actual donor plug, including the empty pin-8 position.

No V2 PCB source, pin mapping, BOM or ordering package exists yet. Those directories will be created only when V2 design work begins.
