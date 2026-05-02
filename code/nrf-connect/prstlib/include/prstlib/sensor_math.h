#ifndef _PRST_SENSOR_MATH_H_
#define _PRST_SENSOR_MATH_H_

#include <stdint.h>

float prst_battery_percent_from_voltage(float voltage);

float prst_soil_calibration_poly_eval(const int coeffs[3], float battery_voltage);

float prst_soil_moisture_percent_from_adc(float battery_voltage,
                                          int16_t raw_adc_output,
                                          const int dry_coeffs[3],
                                          const int wet_coeffs[3]);

uint16_t prst_phototransistor_brightness_from_voltage(float adc_voltage,
                                                      float output_ohms);

uint16_t prst_ldr_brightness_from_voltage(float battery_voltage,
                                          float adc_voltage,
                                          float output_ohms);

#endif  // _PRST_SENSOR_MATH_H_
