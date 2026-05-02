# Release Checklist

Use this checklist for firmware or hardware releases. Keep release notes factual: what was built, what was tested, what changed, and what remains experimental.

## Firmware Release

1. Start from a clean branch with only intended changes.
2. Confirm the pinned nRF Connect SDK Docker image in `.github/actions/build/action.yml` and `.devcontainer/devcontainer.json`.
3. Run the firmware CI workflow on the release branch.
4. Confirm artifacts exist for:
   - `blinky_nrf52840.hex`
   - `input_nrf52840.hex`
   - `soil_read_loop_nrf52840.hex`
   - BLE `nrf52840` and `nrf52833` builds for revisions `1.0.0`, `1.1.0`, `1.2.0`, and `2.0.0`
   - Zigbee `nrf52840` and `nrf52833` builds for revisions `1.2.0` and `2.0.0`
5. Generate checksums for every released `.hex` artifact.
6. Flash the BLE BTHome v2 image on at least one physical board.
7. Verify Home Assistant discovers the device through BTHome v2 and reports:
   - Temperature
   - Humidity
   - Soil moisture
   - Battery voltage and percentage
   - Illuminance, when the board revision has a light sensor
8. Verify a board revision without a light sensor advertises valid data and does not publish garbage illuminance values.
9. Power-profile at least one BLE build after changes that touch sleep, sensing, advertising, GPIO, PWM, I2C, or ADC behavior.
10. If Zigbee is included in the release, flash a board, join it to a coordinator, verify cluster updates, and test the configured factory-reset method.
11. Document known caveats, supported board revisions, and compatibility-breaking changes in the release notes.

## Hardware Release

1. Open the KiCad project with the release KiCad version and save files intentionally.
2. Run ERC and DRC. Record any waivers with a short explanation.
3. Regenerate:
   - Schematic PDF
   - Gerbers
   - Drill files
   - BOM
   - Pick-and-place file
   - Board render or photos
4. Check BOM fields for manufacturer part number, supplier part number, LCSC/JLC code, quantity, side, and DNP status.
5. Confirm polarity and orientation for LED, diode, SHTC3 or successor sensor, module, and battery holder.
6. Confirm test pads and programming access are reachable in the intended case.
7. Upload fabrication files to the intended manufacturer and resolve all parser warnings before tagging.
8. Build first articles before marking the hardware release final.
9. On first articles, verify:
   - SWD flashing
   - Boot and BLE advertisement
   - Temperature and humidity readings
   - Soil sensor dry and wet readings
   - Battery voltage reading
   - Light reading, when populated
   - Sleep current
   - Active current during sensing and radio activity
10. Publish exact fabrication files, KiCad version, assembly notes, and known hardware caveats.

## Release Notes Template

```markdown
# b-parasite <version>

## Firmware

- SDK image: `<docker image reference>`
- Supported SoCs:
- Supported board revisions:
- Built artifacts:
- Checksums:

## Hardware

- KiCad version:
- Fabrication package:
- BOM revision:

## Validation

- CI:
- Physical boards tested:
- Home Assistant / BTHome:
- Zigbee, if applicable:
- Power profile:

## Known Caveats

-
```
