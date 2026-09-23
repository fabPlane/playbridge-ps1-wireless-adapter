# Current images

These are the visually reviewed renders of the electrically complete 55 x 52 mm revision generated on 2026-09-17.

Routing is complete with zero DRC errors and zero unconnected items; 25 physical DRC warnings remain documented. ERC is 0/0. The provisional nine-pin model has the corrected 0.3937007874 WRL scale and outward orientation.

The six images in this folder were generated from a disposable visualization copy using the verified J2 USB-C model transform: offset `(-4.45, -3.65, 0)`, rotation `(-90, 0, 0)`, scale `(1, 1, 1)`. They show the intended final appearance, but do not prove that the production `board.kicad_pcb` has synchronized that 3D-only transform. PCB land geometry and routing were not changed for these renders.

Connector correction found after these renders: the standard controller plug has eight metal contacts across nine positions, with pin 8 (second from the right when looking directly at the male mating face) physically absent. These images still show nine provisional pins and must not be used as the final J1 population reference.

- `full-board-top-orthographic.png`
- `full-board-bottom-orthographic.png`
- `full-board-perspective.png`
- `usb-c-edge-close-up.png`
- `usb-c-top-close-up.png`
- `usb-c-close-perspective.png`
