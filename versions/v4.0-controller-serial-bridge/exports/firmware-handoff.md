# NewPlayBridge2 firmware handoff

## Verified ESP32 GPIO map

| Function | GPIO | ESP32 module pad | Direction |
| --- | ---: | ---: | --- |
| PS1 CMD | 19 | 31 | Output through R7, 1 kOhm |
| PS1 CLK | 32 | 8 | Output through R9, 1 kOhm |
| PS1 ATT | 21 | 33 | Output through R21/R22 and R10, 1 kOhm interface resistor |
| PS1 DATA drive | 33 | 9 | Output through 100 Ohm gate resistor R8 to Q1 |
| PS1 ACK drive | 27 | 12 | Output through 100 Ohm gate resistor R37 to Q2 |
| PS1 ACK sense | 34 | 6 | Input through R11 |
| Status LED | 25 | 10 | Output through R42 to D9 |
| UART2 TX | 17 | 28 | Output through R14, 1 kOhm, to J2 pin 1 |
| UART2 RX | 16 | 27 | Input through R15, 1 kOhm, from J2 pin 4 |
| UART0 TX | 1 | 35 | CH340C programming/debug only |
| UART0 RX | 3 | 34 | CH340C programming/debug only |

GPIO13/module pad 16 and GPIO5/module pad 29 are unconnected. They must not be
used as aliases for the status LED or ACK drive.

Q1 and Q2 are 2N7002 open-drain stages with pin 1 = gate, pin 2 = source/GND,
and pin 3 = drain. Firmware drives the gates high to pull the corresponding
PlayStation line low and drives them low to release the line.

## BOOT/RESET behavior

Manual switches are supported and are the accepted programming fallback:

- SW1 pulls `ESP_EN` low for reset.
- SW2 pulls `ESP_BOOT`/GPIO0 low for bootloader entry.

The existing DTR/RTS-assisted topology is retained without redesign:

- DTR passes through R39 to Q4 base; Q4 emitter is RTS and collector is
  `ESP_EN`.
- RTS passes through R40 to Q5 base; Q5 emitter is DTR and collector is
  `ESP_BOOT`/GPIO0.
- Q4 and Q5 are MMBT3904 devices with pin 1 = base, pin 2 = emitter, and
  pin 3 = collector.

Do not claim automatic programming qualification until it is verified on an
assembled board. Manual BOOT/RESET remains the approved bring-up method.

