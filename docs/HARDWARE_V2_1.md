# Hardware v2.1 Board Backlog

Snapshot date: 2026-05-02.

This is the working checklist for a v2.1 maintenance board. It is based on the current v2.0.0 KiCad design files and intentionally avoids a broad redesign. Keep the nRF52 module, CR2032 power model, sensor set, board outline, electrode geometry, and case compatibility unless a measured defect proves that one of those constraints has to change.

## Current Board Evidence

The current KiCad project and PCB are marked revision `2.0.0`.

Existing programming and test access is better than the release docs previously implied:

|Reference|Net or function|PCB position|State|
|---|---|---:|---|
|`TP8`|`P0.10/NFC2` / net `0.10`|`77.382, 29.628`|Present|
|`TP7`|`+3V0` / label `VCC`|`77.382, 32.168`|Present|
|`TP1`|`SWDIO` / label `IO`|`77.382, 34.708`|Present|
|`TP2`|`SWDCLK` / label `CLK`|`77.382, 37.248`|Present|
|`TP3`|`GND`|`77.362, 39.788`|Present|
|`TP4`|`RST`|`77.382, 42.328`|Present|
|`TP5`|`CPARA`|`67.984, 58.838`|Present|
|`TP6`|`PWM`|`61.38, 59.473`|Present|

Other relevant observations:

- `BT1` is a Keystone 3002 CR2032 holder. Its positive pads are on `+3V0` and its negative pad is on `GND`; no separate battery-current break, shipping disconnect, or raw battery net was found.
- `SDA` and `SCL` are routed between the E73 module and SHTC3, but there are no dedicated I2C test pads.
- The E73 module exposes spare no-connect pins. `P0.31/AIN7` is currently unconnected and is a good first candidate for a low-leakage board-revision strap if the final layout and firmware agree.
- The design has only partial BOM normalization. Many parts have `LCSC` fields, but full manufacturer, supplier, side, quantity, DNP, and alternate-part metadata is not consistently captured.
- No explicit conformal-coating, programming-fixture, humidity-sensor keepout, antenna keepout, or battery-contact keepout documentation was found in the repo.

## Feature Decisions

|Feature|Decision|Action|
|---|---|---|
|SWD, reset, power, and ground access|Already present|Keep the existing vertical row. For v2.1, improve labels and verify pogo-pin reach in the intended case or fixture.|
|Standard Tag-Connect footprint|Evaluate for v2.1|Do not require it. Add only if it fits without crowding the antenna, case, battery holder, or existing test row. The existing row is already a workable pogo target.|
|I2C test access|Add in v2.1|Add `SDA` and `SCL` pads, preferably near the existing test row or SHTC3. Do not create a user-facing expansion connector unless there is a specific accessory requirement.|
|Battery-current measurement|Add in v2.1|Split the battery positive path into `VBAT_RAW` and the board rail, then bridge it with a normally populated 0 ohm resistor or solder jumper that can be lifted for ammeter insertion.|
|Shipping disconnect|Evaluate for v2.1|Prefer a mechanical pull tab or normally closed solder bridge only if it does not make battery insertion worse. Avoid active power switches and avoid adding quiescent load.|
|Hardware revision autodetect|Add in v2.1|Use a spare low-leakage GPIO or ADC-capable pin, with `P0.31/AIN7` as the first candidate. Use high-value or DNP resistor options and verify sleep-current impact before release.|
|Reservoir capacitance option|Evaluate for v2.1|Add DNP capacitor footprints near module `VDD`/`VDDH` only if bench data shows CR2032 sag or radio-burst instability. Do not populate high-leakage bulk parts by default.|
|Conformal-coating and fab keepouts|Add in v2.1|Document and, where practical, draw keepouts for SHTC3, light sensor aperture, soil electrode area, battery contacts, programming pads, and module antenna region.|
|Assembly fiducials and polarity clarity|Add in v2.1|Add global fiducials if the chosen assembler needs them. Improve diode, LED, battery, and sensor orientation cues on fab or silkscreen layers.|
|BOM and assembly metadata|Add in v2.1|Normalize fields for manufacturer part number, supplier part number, LCSC/JLC code, quantity, side, DNP, and approved alternates.|
|Sensor airflow and coating strategy|Evaluate for v2.1|Preserve SHTC3 unless field data shows a sensor defect. Improve case and coating notes first; measure before moving the humidity sensor.|
|Soil electrode geometry changes|Do not add to v2.1 by default|Changing the electrode shape would invalidate calibration assumptions. Treat any electrode redesign as its own measured experiment.|
|SHT4x or SHT45 humidity sensor|Prototype for v3.0|Do not dual-footprint the maintenance board unless supply risk demands it. Prototype separately and compare accuracy, power, condensation recovery, assembly support, and firmware work.|
|nRF54L module or SoC migration|Prototype for v3.0|Do not mix this into v2.1. It changes RF, SDK, module availability, power validation, and certification assumptions.|
|Rechargeable LiPo, USB-C, display, buzzer, or always-on fuel gauge|Do not add now|These add mechanical size, quiescent load, support burden, and failure modes that conflict with the low-power CR2032 product shape.|

## v2.1 Implementation Order

1. Branch the KiCad work separately from firmware-only work.
2. Preserve the v2.0.0 outline, electrode geometry, module footprint, battery holder, SHTC3, ALS-PT19, and case-critical mechanical references.
3. Rename or annotate the existing programming row so the fixture map is explicit: `VCC`, `SWDIO`, `SWDCLK`, `GND`, `RST`, plus the existing spare `P0.10` pad.
4. Add `SDA` and `SCL` test pads with short, low-risk routing. Keep them probeable after conformal coating decisions are made.
5. Split `BT1+` from `+3V0` through a current-measurement jumper or 0 ohm resistor. The release board should ship closed; lab units should allow ammeter insertion without cutting copper.
6. Decide whether the same battery split can also support shipping disconnect. If not, document a battery pull-tab approach instead of adding a second power feature.
7. Add the board-revision strap and firmware mapping in the same change set. The hardware is incomplete until firmware can read and report the revision without hurting sleep current.
8. Add optional DNP reservoir capacitor footprints only after a placement review confirms they do not hurt RF, sensor exposure, or battery clearance.
9. Add keepout drawings and assembly notes for coating, antenna clearance, programming pads, battery contacts, and the soil electrode area.
10. Normalize schematic fields and generated BOM output before fabrication files are regenerated.

## Acceptance Criteria

Before calling v2.1 ready for first articles:

- ERC and DRC are clean or every waiver is documented.
- Fabrication upload accepts Gerbers, drills, pick-and-place, and BOM without unresolved parser warnings.
- The programming fixture can reach `VCC`, `SWDIO`, `SWDCLK`, `GND`, and `RST` with the intended case or no-case assembly state.
- The current-measurement path can be opened and closed repeatedly without damaging traces or making normal assembly ambiguous.
- Battery insertion, removal, and any shipping-disconnect mechanism work with the selected case.
- Firmware can read the hardware revision strap and still meet the sleep-current budget.
- SHTC3 humidity readings, ALS-PT19 light readings, soil dry/wet readings, BLE advertisements, reset behavior, sleep current, and active current are validated on first articles.
- Board photos or renders, schematic PDF, Gerbers, drill files, BOM, pick-and-place, KiCad version, and known caveats are published with the hardware release.
