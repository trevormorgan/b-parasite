#include "prstlib/adc.h"

#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "prstlib/macros.h"
#include "prstlib/sensor_math.h"

LOG_MODULE_REGISTER(adc, CONFIG_PRSTLIB_LOG_LEVEL);

// PWM spec for square wave. Input to the soil sensing circuit.
static const struct pwm_dt_spec soil_pwm_dt =
    PWM_DT_SPEC_GET(DT_NODELABEL(soil_pwm));
static const uint32_t pulse = DT_PROP(DT_NODELABEL(soil_pwm), pulse);

// Calibration coefficients for the soil sensing circuit.
static const int dry_coeffs[3] = DT_PROP(DT_NODELABEL(soil_calibration_coeffs), dry);
static const int wet_coeffs[3] = DT_PROP(DT_NODELABEL(soil_calibration_coeffs), wet);

struct gpio_dt_spec fast_disch_dt =
    GPIO_DT_SPEC_GET(DT_NODELABEL(fast_disch), gpios);

// Shared buffer and adc_sequennce.
static int16_t buf;
static struct adc_sequence sequence = {
    .buffer = &buf,
    .buffer_size = sizeof(buf),
};

static const struct adc_dt_spec adc_soil_spec =
    ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 0);

static const struct adc_dt_spec adc_batt_spec =
    ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 1);

#if DT_NODE_EXISTS(DT_NODELABEL(photo_transistor))

static const struct adc_dt_spec adc_photo_transistor_spec =
    ADC_DT_SPEC_GET(DT_NODELABEL(photo_transistor));
struct gpio_dt_spec photo_transistor_enable_dt =
    GPIO_DT_SPEC_GET(DT_NODELABEL(photo_transistor_enable), gpios);

#elif DT_NODE_EXISTS(DT_NODELABEL(ldr))

static const struct adc_dt_spec adc_ldr_spec =
    ADC_DT_SPEC_GET(DT_NODELABEL(ldr));
struct gpio_dt_spec ldr_enable_dt =
    GPIO_DT_SPEC_GET(DT_NODELABEL(ldr_enable), gpios);

#endif

static int read_adc_spec(const struct adc_dt_spec* spec, prst_adc_read_t* out) {
  RET_IF_ERR(adc_sequence_init_dt(spec, &sequence));

  RET_IF_ERR(adc_read(spec->dev, &sequence));

  int32_t val_mv = buf;
  RET_IF_ERR(adc_raw_to_millivolts_dt(spec, &val_mv));
  val_mv = MAX(0, val_mv);

  out->raw = buf;
  out->millivolts = val_mv;
  out->voltage = val_mv / 1000.0f;
  return 0;
}

int prst_adc_init() {
  RET_IF_ERR(adc_channel_setup_dt(&adc_soil_spec));
  RET_IF_ERR(adc_channel_setup_dt(&adc_batt_spec));

  RET_IF_ERR(!device_is_ready(fast_disch_dt.port));
  RET_IF_ERR(gpio_pin_configure_dt(&fast_disch_dt, GPIO_OUTPUT));

#if DT_NODE_EXISTS(DT_NODELABEL(photo_transistor))
  RET_IF_ERR(adc_channel_setup_dt(&adc_photo_transistor_spec));
  RET_IF_ERR(!device_is_ready(photo_transistor_enable_dt.port));
  RET_IF_ERR(gpio_pin_configure_dt(&photo_transistor_enable_dt, GPIO_OUTPUT));
#elif DT_NODE_EXISTS(DT_NODELABEL(ldr))
  RET_IF_ERR(adc_channel_setup_dt(&adc_ldr_spec));
  RET_IF_ERR(!device_is_ready(ldr_enable_dt.port));
  RET_IF_ERR(gpio_pin_configure_dt(&ldr_enable_dt, GPIO_OUTPUT));
#endif

  for (size_t idx = 0; idx < ARRAY_SIZE(dry_coeffs); idx++) {
    LOG_DBG("Dry coeff %d: %d\n", idx, dry_coeffs[idx]);
  }
  for (size_t idx = 0; idx < ARRAY_SIZE(wet_coeffs); idx++) {
    LOG_DBG("Wet coeff %d: %d\n", idx, wet_coeffs[idx]);
  }
  return 0;
}

int prst_adc_batt_read(prst_batt_t* out) {
  RET_IF_ERR(read_adc_spec(&adc_batt_spec, &out->adc_read));
  out->percentage = prst_battery_percent_from_voltage(out->adc_read.voltage);
  return 0;
}

int prst_adc_soil_read(float battery_voltage, prst_adc_soil_moisture_t* out) {
  // Enable fast discharge circuit.
  RET_IF_ERR(gpio_pin_set_dt(&fast_disch_dt, 1));
  // Start PWM.
  int err = pwm_set_dt(&soil_pwm_dt, soil_pwm_dt.period, pulse);
  if (err) {
    RET_IF_ERR(gpio_pin_set_dt(&fast_disch_dt, 0));
    return err;
  }
  k_msleep(30);

  err = read_adc_spec(&adc_soil_spec, &out->adc_read);

  // Stop PWM.
  int cleanup_err = pwm_set_dt(&soil_pwm_dt, 0, 0);
  if (cleanup_err) {
    LOG_ERR("Error %d stopping soil PWM", cleanup_err);
    if (!err) {
      err = cleanup_err;
    }
  }
  // Turn off fast discharge circuit.
  cleanup_err = gpio_pin_set_dt(&fast_disch_dt, 0);
  if (cleanup_err) {
    LOG_ERR("Error %d disabling fast discharge", cleanup_err);
    if (!err) {
      err = cleanup_err;
    }
  }
  if (err) {
    return err;
  }

  const float dry = prst_soil_calibration_poly_eval(dry_coeffs, battery_voltage);
  const float wet = prst_soil_calibration_poly_eval(wet_coeffs, battery_voltage);
  const float percent = prst_soil_moisture_percent_from_adc(
      battery_voltage, buf, dry_coeffs, wet_coeffs);
  LOG_DBG("Read soil moisture 2: %.2f | Raw %u | Batt: %.2f | Dry: %.2f | Wet: %.2f",
          DOUBLE_PROMO_OK(100 * percent),
          buf, DOUBLE_PROMO_OK(battery_voltage), DOUBLE_PROMO_OK(dry),
          DOUBLE_PROMO_OK(wet));

  out->percentage = MAX(0.0f, MIN(1.0f, percent));
  return 0;
}

int prst_adc_photo_read(float battery_voltage, prst_adc_photo_sensor_t* out) {
#if DT_NODE_EXISTS(DT_NODELABEL(photo_transistor))
  UNUSED_OK(battery_voltage);
  RET_IF_ERR(gpio_pin_set_dt(&photo_transistor_enable_dt, 1));
  k_msleep(10);
  int err = read_adc_spec(&adc_photo_transistor_spec, &out->adc_read);
  int cleanup_err = gpio_pin_set_dt(&photo_transistor_enable_dt, 0);
  if (cleanup_err) {
    LOG_ERR("Error %d disabling phototransistor", cleanup_err);
    if (!err) {
      err = cleanup_err;
    }
  }
  if (err) {
    return err;
  }

  const float phototransistor_resistor = DT_PROP(DT_NODELABEL(photo_transistor), output_ohms);
  out->brightness = prst_phototransistor_brightness_from_voltage(
      out->adc_read.voltage, phototransistor_resistor);
  LOG_DBG("Read phototransistor: %u lx | %.2f V", out->brightness, DOUBLE_PROMO_OK(out->adc_read.voltage));

#elif DT_NODE_EXISTS(DT_NODELABEL(ldr))
  RET_IF_ERR(gpio_pin_set_dt(&ldr_enable_dt, 1));
  k_msleep(10);
  int err = read_adc_spec(&adc_ldr_spec, &out->adc_read);
  int cleanup_err = gpio_pin_set_dt(&ldr_enable_dt, 0);
  if (cleanup_err) {
    LOG_ERR("Error %d disabling LDR", cleanup_err);
    if (!err) {
      err = cleanup_err;
    }
  }
  if (err) {
    return err;
  }

  // The photo resistor forms a voltage divider with R.
  // The voltage here is measured in the middle of the voltage divider.
  // Vcc ---- (R_photo) ---|--- (R) ---- GND
  //                      Vout
  // So we can estimate R_photo = R * (Vcc - Vout) / Vout
  const float r = DT_PROP(DT_NODELABEL(ldr), output_ohms);
  out->brightness = prst_ldr_brightness_from_voltage(
      battery_voltage, out->adc_read.voltage, r);
  LOG_DBG("Read LDR: %u lx | %.2f V", out->brightness, DOUBLE_PROMO_OK(out->adc_read.voltage));

#else
  UNUSED_OK(battery_voltage);
  out->adc_read.raw = 0;
  out->adc_read.millivolts = 0;
  out->adc_read.voltage = 0.0f;
  out->brightness = 0;
  LOG_DBG("No photo sensor configured");
#endif

  return 0;
}
