# DO NOT ORDER — sourcing and PCBA completeness review

The connector-orientation and routing hold is resolved. J1 and J2 are installed at their approved
lower-edge placements with both mating mouths facing outward, and the authoritative board passes ERC,
DRC, connectivity, and schematic-parity checks with zero findings.

Keep this ordering hold only until the following commercial checks are completed at checkout:

- Reconfirm live JLCPCB/LCSC availability for Molex 48393-0003 / C3197928. Two are required per board;
  the last public check showed only seven units.
- Reconfirm availability of the low-stock 4.7 uH inductor L1, MYX MCNR4030AF-4R7M / C49449313.
- Review the uploaded BOM and CPL for 63 fitted references and four DNP bare-board test pads
  (TP1–TP4), including rotation previews and any assembler substitutions.
- Confirm the assembler accepts every project-local/custom footprint and the explicitly reserved,
  all-NC J2 population.

This is a sourcing/assembly-completeness hold, not a connector-orientation or electrical-layout hold.
