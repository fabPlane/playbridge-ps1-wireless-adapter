# Accepted V3.0 J2 serial verification renders

- `v3.0-populated-top.png`: complete populated top view with board routing,
  component models, `J2 PS1 SERIAL`, STATUS/PWR LEDs, switches and UART0 service
  holes visible.
- `v3.0-populated-perspective.png`: populated perspective view proving the J1
  and J2 connector bodies are seated at the lower edge and face outward.
- `v3.0-j1-j2-edge.png`: direct lower-edge view into both J1 and J2 mouths.
- `v3.0-bottom.png`: full bottom copper, pad and via inspection view.

These images were regenerated after replacing the unavailable C3197928 with
ready-for-assembly Hong Cheng HC-USB3.0-L168-ZP / JLCPCB C7501856. The nine
electrical-pad centers remain unchanged, so no copper was rerouted. The new
official vendor model, two shell slots and body/courtyard are shown. Final DRC
remains zero errors, zero unconnected pads and zero schematic-parity findings,
with only the three inherited locally approved UART0 test-stub warnings.

SHA-256:

```text
7430235c1d6ccdc6e1f984870e74b5c252d18815d5336b8645e68bc4859c599c  v3.0-bottom.png
3348143ce81ccffe45ece28a55ce5dd7a45624dd9e076883bae2e75fa52c4568  v3.0-j1-j2-edge.png
e473214ecfee678085f4d2535282d27b7a868cd929821892c8ba9b2b4f12acbb  v3.0-populated-perspective.png
4f17eef1b52b5d0e5f0a0a7df80283226a352747681b3990957ca9d529a8d80f  v3.0-populated-top.png
```

The older V2.1 service-debug images remain as inherited comparison artifacts;
the four `v3.0-*.png` images above are the V3.0 release-review set.
