#include "sensor_attrs.h"

#include <stdio.h>

static int failures;

#define CHECK(expr)                                                            \
  do {                                                                         \
    if (!(expr)) {                                                             \
      fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #expr); \
      failures++;                                                              \
    }                                                                          \
  } while (0)

int main(void) {
  CHECK(prst_zb_encode_illuminance_measured_value(0) ==
        PRST_ZB_ILLUMINANCE_MEASURED_VALUE_UNKNOWN);
  CHECK(prst_zb_encode_illuminance_measured_value(1) == 1);
  CHECK(prst_zb_encode_illuminance_measured_value(10) == 10001);
  CHECK(prst_zb_encode_illuminance_measured_value(1000) == 30001);

  return failures == 0 ? 0 : 1;
}
