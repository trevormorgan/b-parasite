# Modernization Plan

Snapshot date: 2026-05-02.

This plan keeps the project shippable while moving the software, hardware, and release process toward current tooling. The current production baseline remains the nRF52-based hardware with the BLE BTHome v2 firmware; larger SoC and sensor changes should be proven on prototypes before replacing that baseline.

## Current State

- Firmware builds with Nordic nRF Connect SDK 2.7 in Docker. CI and the devcontainer now pin the SDK container by digest to avoid accidental toolchain drift.
- BLE is the primary firmware path. It supports BTHome v2, legacy BTHome v1, and the older b-parasite ESPHome encoding.
- Zigbee is useful but still experimental. It needs explicit compatibility tests before being treated as a production firmware.
- Hardware design files are KiCad 7-era artifacts, with generated Gerbers from KiCad 7.0.1.
- The main hardware platform is still sound for a CR2032 soil sensor: nRF52840 or nRF52833 module, SHTC3, ALS-PT19, and capacitive soil sensing.

## Software Track

1. Keep the nRF Connect SDK 2.7 branch as the release baseline until the full build matrix is green on a newer SDK.
2. Create a migration branch for nRF Connect SDK 3.x. Nordic's April 2026 SDK 3.3 release adds broader nRF54 support and continues the move away from Partition Manager toward devicetree-based flash partitioning for multi-image builds.
3. Move any multi-image or bootloader work toward sysbuild and devicetree partitioning as part of the SDK 3.x migration.
4. Treat BLE BTHome v2 as the default production firmware. Keep legacy encodings buildable for compatibility, but avoid new features in legacy packet formats.
5. Add host-side tests for:
   - BTHome v2 packet encoding fixtures.
   - Battery percentage and voltage conversion edges.
   - Soil calibration math.
   - Illuminance encoding, especially zero and saturation cases.
   - Double-reset flag parsing.
6. Add firmware behavior tests with Zephyr ztest where hardware is not required.
7. Add a small hardware-in-loop smoke test later: flash one nRF52840 board, read one BLE advertisement, and verify decoded temperature, humidity, soil, battery, and light fields.
8. Replace normal field-failure asserts with explicit retries, error counters, and last-known-good behavior where the device can safely continue.
9. Modernize Python flashing tools with a small `pyproject.toml`, formatting, linting, and a single command interface after the firmware release path is stable.
10. Decide whether Zigbee remains experimental or becomes a supported target. If it becomes supported, add Home Assistant ZHA and Zigbee2MQTT converter acceptance tests before advertising it as production-ready.

## Hardware Track

1. Cut a v2.1 maintenance board before a v3.0 redesign. Keep the nRF52 module footprint and existing mechanical envelope unless a specific defect requires otherwise.
2. For v2.1, prioritize manufacturing and field reliability:
   - Add or improve test pads for power, reset, SWD, I2C, soil drive, soil sense, and key GPIOs.
   - Add a clear shipping or battery-disconnect option if it can be done without hurting size or cost.
   - Re-check CR2032 reservoir capacitance and brownout margin during radio bursts.
   - Define coating keepouts around the humidity sensor, electrode area, connector/test pads, battery contacts, and antenna.
   - Improve BOM metadata: manufacturer part number, supplier part number, LCSC/JLC code, quantity, side, DNP status, and alternates.
3. Migrate the KiCad project to KiCad 10 on a dedicated branch. Regenerate schematic PDF, Gerbers, drill files, pick-and-place, and BOM from the migrated project.
4. Add automated ERC/DRC and fabrication export once the KiCad 10 migration is stable, preferably through KiBot or another reproducible KiCad CLI flow.
5. Prototype a v3.0 sensor stack separately:
   - Evaluate SHT4x, especially SHT45, for better accuracy and lower voltage operation.
   - Evaluate nRF54L only on a prototype board. Nordic presents nRF54L15 as an nRF52 successor with lower power for common BLE use cases, but b-parasite needs measured CR2032 results before changing the production SoC.
   - Compare module availability, antenna certification, JLC assembly support, sleep current, RF performance, and SDK maturity against the current nRF52 modules.
6. Run environmental tests before any hardware release:
   - Dry air, humid air, condensation recovery, and soil splash exposure.
   - Electrode corrosion and drift over repeated wet/dry cycles.
   - Case airflow versus splash protection.
   - Battery sag across temperature and radio burst profiles.

## Release Track

1. Keep CI green for every supported SoC and board revision before tagging.
2. Upload firmware hex artifacts for each matrix entry, with checksums in the release notes.
3. Flash at least one nRF52840 board and one nRF52833 board before a firmware release if both SoCs are advertised.
4. Validate BLE discovery in Home Assistant through BTHome v2 before release.
5. For hardware releases, publish schematic PDF, Gerbers, drill files, BOM, pick-and-place, board photos/renders, and the exact KiCad version used.
6. Keep the release notes explicit about supported board revisions, firmware sample maturity, known caveats, and migration instructions.

## Decision Gates

|Gate|Exit Criteria|
|---|---|
|SDK 3.x migration|All firmware samples build for supported SoCs/revisions, no deprecated build flow remains for supported targets, BLE BTHome v2 packets match fixtures.|
|Zigbee production support|ZHA and Zigbee2MQTT paths are documented and tested, reset behavior is reliable on all supported revisions, power profile is measured.|
|KiCad 10 migration|ERC/DRC accepted or waived with notes, generated fabrication files match expected board outline/layers, BOM and placement files are complete.|
|v2.1 hardware|Assembly files accepted by the chosen fab, first articles flash successfully, sensor readings are plausible, sleep and active current match budget.|
|v3.0 hardware|nRF54L and SHT4x prototypes beat the v2.x baseline on measured battery life, reliability, availability, and integration cost.|

## References

- Nordic nRF Connect SDK 3.3 release notes: https://devzone.nordicsemi.com/nordic/nordic-blog/b/blog/posts/whats-new-in-nrf-connect-sdk-v3-3-0
- Zephyr supported releases: https://docs.zephyrproject.org/latest/releases/index.html
- KiCad 10.0.1 release: https://www.kicad.org/blog/2026/04/KiCad-10.0.1-Release/
- BTHome v2 format: https://bthome.io/format/
- BTHome encryption notes: https://bthome.io/encryption/
- Nordic nRF54L15 product page: https://www.nordicsemi.com/Products/nRF54L15
- Sensirion SHT45 product page: https://sensirion.com/products/catalog/SHT45
