#include "encoding.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(expr)                                                            \
  do {                                                                         \
    if (!(expr)) {                                                             \
      fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #expr); \
      failures++;                                                              \
    }                                                                          \
  } while (0)

int main(void) {
  prst_sensors_t sensors = {
      .soil = {.percentage = 0.321f},
      .photo = {.brightness = 1234},
      .batt =
          {
              .adc_read = {.millivolts = 2987, .voltage = 2.987f},
              .percentage = 0.435f,
          },
      .shtc3 = {.temp_c = 21.34f, .rel_humi = 0.567f},
  };
  bt_addr_le_t bt_addr = {.type = 0, .a = {.val = {1, 2, 3, 4, 5, 6}}};

  uint8_t out[CONFIG_PRST_BLE_ENCODING_SERVICE_DATA_LEN] = {0};
  const uint8_t expected[] = {
      0xd2, 0xfc, 0x40, 0x01, 0x2c, 0x02, 0x56, 0x08, 0x05, 0x08,
      0xe2, 0x01, 0x0c, 0xab, 0x0b, 0x2e, 0x39, 0x2f, 0x20,
  };

  CHECK(prst_ble_encode_service_data(&sensors, &bt_addr, out, sizeof(out)) ==
        0);
  CHECK(sizeof(out) == sizeof(expected));
  CHECK(memcmp(out, expected, sizeof(expected)) == 0);

  uint8_t short_out[CONFIG_PRST_BLE_ENCODING_SERVICE_DATA_LEN - 1] = {0};
  CHECK(prst_ble_encode_service_data(&sensors, &bt_addr, short_out,
                                     sizeof(short_out)) != 0);

  return failures == 0 ? 0 : 1;
}
