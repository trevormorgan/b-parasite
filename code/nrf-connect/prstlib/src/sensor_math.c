#include "prstlib/sensor_math.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  // High (h) and low (p) voltage (v) and % (p) points.
  float vh, vl, ph, pl;
} batt_disch_linear_section_t;

static uint16_t saturating_lux(float lux) {
  if (lux <= 0.0f) {
    return 0;
  }
  if (lux >= UINT16_MAX) {
    return UINT16_MAX;
  }
  return (uint16_t)lux;
}

float prst_battery_percent_from_voltage(float voltage) {
  // Must be sorted by .vh.
  static const batt_disch_linear_section_t sections[] = {
      {.vh = 3.00f, .vl = 2.90f, .ph = 1.00f, .pl = 0.42f},
      {.vh = 2.90f, .vl = 2.74f, .ph = 0.42f, .pl = 0.18f},
      {.vh = 2.74f, .vl = 2.44f, .ph = 0.18f, .pl = 0.06f},
      {.vh = 2.44f, .vl = 2.01f, .ph = 0.06f, .pl = 0.00f},
  };

  if (voltage > sections[0].vh) {
    return 1.0f;
  }
  for (size_t i = 0; i < sizeof(sections) / sizeof(sections[0]); i++) {
    const batt_disch_linear_section_t* s = &sections[i];
    if (voltage > s->vl) {
      return s->pl + (voltage - s->vl) * ((s->ph - s->pl) / (s->vh - s->vl));
    }
  }
  return 0.0f;
}

float prst_soil_calibration_poly_eval(const int coeffs[3], float battery_voltage) {
  // The coefficients are specified times 1000, as a workaround for the lack of
  // floating point support in devicetree bindings.
  return (coeffs[0] + coeffs[1] * battery_voltage +
          coeffs[2] * battery_voltage * battery_voltage) /
         1000.0f;
}

float prst_soil_moisture_percent_from_adc(float battery_voltage,
                                          int16_t raw_adc_output,
                                          const int dry_coeffs[3],
                                          const int wet_coeffs[3]) {
  const float dry = prst_soil_calibration_poly_eval(dry_coeffs, battery_voltage);
  const float wet = prst_soil_calibration_poly_eval(wet_coeffs, battery_voltage);
  return (raw_adc_output - dry) / (wet - dry);
}

uint16_t prst_phototransistor_brightness_from_voltage(float adc_voltage,
                                                      float output_ohms) {
  // Assuming 10000 lux for the saturation test. Calibration with a proper light
  // meter would be better.
  const float lux_sun = 10000.0f;
  const float current_sun = 3.59e-3f;
  const float current = adc_voltage / output_ohms;
  return saturating_lux(lux_sun * current / current_sun);
}

uint16_t prst_ldr_brightness_from_voltage(float battery_voltage,
                                          float adc_voltage,
                                          float output_ohms) {
  if (adc_voltage <= 0.0f) {
    return 0;
  }
  if (adc_voltage >= battery_voltage) {
    return UINT16_MAX;
  }

  // The photo resistor forms a voltage divider with R:
  // Vcc ---- (R_photo) ---|--- (R) ---- GND
  //                      Vout
  const float photo_resistance =
      output_ohms * (battery_voltage - adc_voltage) / adc_voltage;

  // These values work for the GL5528 LDR and were borrowed from
  // https://github.com/QuentinCG/Arduino-Light-Dependent-Resistor-Library.
  const float mult_value = 32017200.0f;
  const float pow_value = 1.5832f;
  return saturating_lux(mult_value / powf(photo_resistance, pow_value));
}
