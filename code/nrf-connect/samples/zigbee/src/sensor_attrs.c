#include "sensor_attrs.h"

#include <math.h>

uint16_t prst_zb_encode_illuminance_measured_value(uint16_t brightness_lux) {
  if (brightness_lux == 0) {
    return PRST_ZB_ILLUMINANCE_MEASURED_VALUE_UNKNOWN;
  }

  const float encoded = 10000.0f * log10f((float)brightness_lux) + 1.0f;
  if (encoded >= PRST_ZB_ILLUMINANCE_MEASURED_VALUE_MAX) {
    return PRST_ZB_ILLUMINANCE_MEASURED_VALUE_MAX;
  }
  return (uint16_t)encoded;
}
