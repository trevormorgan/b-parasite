#ifndef _HOST_TEST_ZEPHYR_BLUETOOTH_H_
#define _HOST_TEST_ZEPHYR_BLUETOOTH_H_

#include <stdint.h>

#define BT_ADDR_SIZE 6

typedef struct {
  uint8_t val[BT_ADDR_SIZE];
} bt_addr_t;

typedef struct {
  uint8_t type;
  bt_addr_t a;
} bt_addr_le_t;

#endif  // _HOST_TEST_ZEPHYR_BLUETOOTH_H_
