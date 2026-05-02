#include "prstlib/sensor_math.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>

static int failures;

#define CHECK(expr)                                                            \
  do {                                                                         \
    if (!(expr)) {                                                             \
      fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #expr); \
      failures++;                                                              \
    }                                                                          \
  } while (0)

#define CHECK_CLOSE(actual_expr, expected_expr, epsilon)                       \
  do {                                                                         \
    const float actual = (actual_expr);                                        \
    const float expected = (expected_expr);                                    \
    if (fabsf(actual - expected) > (epsilon)) {                                \
      fprintf(stderr, "%s:%d: expected %.6f, got %.6f\n", __FILE__, __LINE__,  \
              (double)expected, (double)actual);                               \
      failures++;                                                              \
    }                                                                          \
  } while (0)

int main(void) {
  CHECK_CLOSE(prst_battery_percent_from_voltage(3.10f), 1.0f, 0.0001f);
  CHECK_CLOSE(prst_battery_percent_from_voltage(3.00f), 1.0f, 0.0001f);
  CHECK_CLOSE(prst_battery_percent_from_voltage(2.90f), 0.42f, 0.0001f);
  CHECK_CLOSE(prst_battery_percent_from_voltage(2.82f), 0.30f, 0.0001f);
  CHECK_CLOSE(prst_battery_percent_from_voltage(2.44f), 0.06f, 0.0001f);
  CHECK_CLOSE(prst_battery_percent_from_voltage(2.01f), 0.0f, 0.0001f);

  const int simple_dry_coeffs[3] = {100000, 0, 0};
  const int simple_wet_coeffs[3] = {200000, 0, 0};
  CHECK_CLOSE(prst_soil_calibration_poly_eval(simple_dry_coeffs, 3.0f), 100.0f,
              0.0001f);
  CHECK_CLOSE(prst_soil_moisture_percent_from_adc(3.0f, 150, simple_dry_coeffs,
                                                  simple_wet_coeffs),
              0.5f, 0.0001f);

  const int v2_dry_coeffs[3] = {334000, 110000, -15300};
  const int v2_wet_coeffs[3] = {299000, -83100, 11200};
  CHECK_CLOSE(prst_soil_calibration_poly_eval(v2_dry_coeffs, 3.0f), 526.3f,
              0.01f);
  CHECK_CLOSE(prst_soil_calibration_poly_eval(v2_wet_coeffs, 3.0f), 150.5f,
              0.01f);

  CHECK(prst_phototransistor_brightness_from_voltage(0.0f, 470.0f) == 0);
  const uint16_t phototransistor_lux =
      prst_phototransistor_brightness_from_voltage(0.16873f, 470.0f);
  CHECK(phototransistor_lux >= 999 && phototransistor_lux <= 1000);
  CHECK(prst_phototransistor_brightness_from_voltage(20.0f, 470.0f) ==
        UINT16_MAX);

  CHECK(prst_ldr_brightness_from_voltage(3.0f, 0.0f, 10000.0f) == 0);
  CHECK(prst_ldr_brightness_from_voltage(3.0f, 1.5f, 10000.0f) == 14);
  CHECK(prst_ldr_brightness_from_voltage(3.0f, 3.0f, 10000.0f) == UINT16_MAX);

  return failures == 0 ? 0 : 1;
}
