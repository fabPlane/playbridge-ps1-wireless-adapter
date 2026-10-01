# NewPlayBridge2 current-limited bench-test plan

Status: **NOT EXECUTED - PRODUCTION HOLD**

No assembled board, programmable supply, DMM, oscilloscope, PlayStation
harness/system, or USB host is connected to this workspace. EDA checks and 3D
renders are not substitutes for the measurements below.

## Pre-power

1. Inspect polarity and soldering at J3, F1/F2, D1/D2, U4/U6, J5, L1,
   Q1/Q2, Q4/Q5, and U1.
2. Confirm no low-resistance short from USB VBUS, J1 pin 8, 5 V, or 3.3 V to
   GND.
3. Confirm continuity from J1 pin 4 and both J1 retention tabs to GND, and
   from J2 pin 7 and both J2 retention tabs to GND.

## USB-only power and programming

1. Apply 5.0 V at J3 with a 100 mA limit for the initial smoke check, then
   raise to 500 mA for ESP32 startup peaks after verifying the rails.
2. Measure protected USB 5 V and 3.3 V; confirm no abnormal heating.
3. Confirm D8 power LED operation and firmware control of D9 status LED.
4. Confirm CH340 enumeration and UART0 logging.
5. Verify programming with the accepted manual sequence: hold BOOT (SW2),
   pulse RESET (SW1), then release BOOT.

The existing DTR/RTS-assisted network may be characterized, but automatic
programming is not a production acceptance requirement because manual
BOOT/RESET is explicitly accepted. Do not describe DTR/RTS operation as
bench-qualified until it is measured on hardware.

## PS1 power and source switching

1. With USB removed, feed the verified PS1 harness voltage into J1 pin 8 from
   a current-limited supply, starting at 100 mA.
2. Verify AP63203, TPS2116, AP2112K, and the final 3.3 V rails.
3. Confirm no reverse voltage appears at USB VBUS.
4. Apply USB and PS1 sources together; verify source selection, stable output,
   and no reverse-current/source-contention fault.

## Interfaces

1. Verify CMD GPIO19, CLK GPIO32, and ATT GPIO21 directions and waveforms.
2. Verify Q1 DATA and Q2 ACK are high impedance when released and pull low
   when GPIO33/GPIO27 assert their gates; verify GPIO34 senses ACK.
3. Verify J2 pin 1 TX from GPIO17, pin 4 RX to GPIO16, and pin 7 GND. Confirm
   J2 causes no activity on UART0 GPIO1/GPIO3.
4. Verify service-hole order GND/TX/RX and 3.2 mm pitch.

## Required recorded results

| Test | Required evidence | Status |
| --- | --- | --- |
| USB current-limited power | Input current and 5 V/3.3 V measurements | NOT EXECUTED |
| PS1 current-limited power | Input current and regulator measurements | NOT EXECUTED |
| Dual-source switching | TPS2116 selection and reverse-current measurements | NOT EXECUTED |
| Manual programming | CH340 enumeration and successful manual BOOT/RESET upload | NOT EXECUTED |
| J1 controller interface | CMD/CLK/ATT and open-drain DATA/ACK captures | NOT EXECUTED |
| J2 serial interface | TX/RX voltage and UART transaction captures | NOT EXECUTED |

Production approval remains blocked until an operator records passing results.
