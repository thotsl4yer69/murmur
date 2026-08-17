# Cirkit import note

This package is intentionally delivered as a Cirkit build-source bundle rather than claiming a proprietary native Cirkit project file format that has not been verified.

Use `CIRKIT_AI_BUILD_SPECIFICATION.md` with Cirkit AI to create the one-project schematic, then load the supplied firmware into the appropriate MCU blocks.

Cirkit's current interface supports blank projects, component construction, custom parts, simulation and firmware upload. A native project serialization/import format is not asserted here without verified documentation.

The bundle therefore remains portable and auditable:
- architecture -> MASTER_BUILD.md
- exact nets -> PHYSICAL_NETLIST.md
- construction -> CIRKIT_AI_BUILD_SPECIFICATION.md
- Bruce -> BRUCE_INTEGRATION.md
- verification -> BUILD_COVERAGE_MATRIX.md + FINAL_DESIGN_REVIEW.md
- upload workflow -> CIRKIT_UPLOAD_GUIDE.md
- controller firmware -> murmur_p01a_controller.ino
- Bruce firmware -> bruce_murmur_node.ino
