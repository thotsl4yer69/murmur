# MURMUR P01-A — Cirkit package

This directory is the handoff package for building the complete P01-A low-voltage simulation/reference design in Cirkit Designer.

## Start here
1. `MASTER_BUILD.md` — architecture and exact GPIO/net map.
2. `CIRKIT_AI_BUILD_SPECIFICATION.md` — paste into Cirkit AI to construct the project.
3. `PHYSICAL_NETLIST.md` — reference component-to-net wiring.
4. `murmur_p01a_controller.ino` — integrated controller firmware.
5. `VERIFICATION_CHECKLIST.md` — electrical, firmware and simulation checks.
6. `FINAL_DESIGN_REVIEW.md` — second-pass review and remaining physical engineering gates.

## Required result
One coherent connected schematic containing controller, sensor bus, haptics, SD, independent timer proofs, interlock/fault paths, controlled low-voltage test load, feedback and test points.

Do not interpret a simulated LED/test load as certification of the production optical/high-power section.
