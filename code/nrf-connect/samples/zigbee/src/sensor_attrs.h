#ifndef _PRST_ZB_SENSOR_ATTRS_H_
#define _PRST_ZB_SENSOR_ATTRS_H_

#include <stdint.h>

#define PRST_ZB_ILLUMINANCE_MEASURED_VALUE_UNKNOWN 0xffff
#define PRST_ZB_ILLUMINANCE_MEASURED_VALUE_MAX 0xfffe

uint16_t prst_zb_encode_illuminance_measured_value(uint16_t brightness_lux);

#endif  // _PRST_ZB_SENSOR_ATTRS_H_
