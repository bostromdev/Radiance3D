#include "hardware_config.hpp"

#include <array>
#include <cstddef>

namespace radiance3d {
namespace {

bool valid_gpio(const int pin) { return pin >= 0 && pin <= 39; }

bool input_only_gpio(const int pin) { return pin >= 34 && pin <= 39; }

bool bootstrapping_gpio(const int pin) {
  return pin == 0 || pin == 2 || pin == 5 || pin == 12 || pin == 15;
}

AxisConfig azimuth_motion() {
  AxisConfig config;
  config.motor_full_steps_per_revolution = 200;
  config.microsteps = 16;
  config.motor_rms_current_ma = 400;
  config.hold_current_percent = 30;
  config.gear_ratio = 1.0;
  config.direction_inverted = false;
  config.home_offset_deg = 0.0;
  config.minimum_angle_deg = 0.0;
  config.maximum_angle_deg = 360.0;
  config.maximum_speed_deg_per_s = 10.0;
  config.acceleration_deg_per_s2 = 20.0;
  config.settling_time_ms = 250;
  config.motion_timeout_ms = 60000;
  config.maximum_bench_test_steps = 3200;
  config.homing.switch_normally_closed = true;
  config.homing.direction_negative = true;
  config.homing.debounce_ms = 10;
  config.homing.speed_deg_per_s = 5.0;
  config.homing.slow_approach_deg_per_s = 1.0;
  config.homing.backoff_deg = 3.0;
  config.homing.timeout_ms = 60000;
  return config;
}

AxisConfig elevation_motion() {
  AxisConfig config = azimuth_motion();
  config.minimum_angle_deg = -90.0;
  config.maximum_angle_deg = 90.0;
  config.maximum_speed_deg_per_s = 8.0;
  config.acceleration_deg_per_s2 = 15.0;
  config.homing.speed_deg_per_s = 4.0;
  return config;
}

}  // namespace

PhysicalControllerConfig provisional_esp32_dev_config() {
  PhysicalControllerConfig config;
  config.board_name = "esp32dev-provisional";
  config.protocol_version = 1;

  config.azimuth.axis.name = "azimuth";
  config.azimuth.axis.motion = azimuth_motion();
  config.azimuth.axis.home_switch_pin = 32;
  config.azimuth.driver.uart_channel = 1;
  config.azimuth.driver.address = 0;
  config.azimuth.driver.uart_tx_pin = 22;
  config.azimuth.driver.uart_rx_pin = 21;
  config.azimuth.driver.step_pin = 25;
  config.azimuth.driver.direction_pin = 26;
  config.azimuth.driver.enable_pin = 27;
  config.azimuth.driver.direction_inverted =
      config.azimuth.axis.motion.direction_inverted;
  config.azimuth.driver.maximum_rms_current_ma = 800;

  config.elevation.axis.name = "elevation";
  config.elevation.axis.motion = elevation_motion();
  config.elevation.axis.home_switch_pin = 33;
  config.elevation.driver.uart_channel = 2;
  config.elevation.driver.address = 0;
  config.elevation.driver.uart_tx_pin = 17;
  config.elevation.driver.uart_rx_pin = 16;
  config.elevation.driver.step_pin = 18;
  config.elevation.driver.direction_pin = 19;
  config.elevation.driver.enable_pin = 23;
  config.elevation.driver.direction_inverted =
      config.elevation.axis.motion.direction_inverted;
  config.elevation.driver.maximum_rms_current_ma = 800;

  config.emergency_stop_pin = 13;
  config.emergency_stop_active_low = true;
  config.emergency_stop_debounce_ms = 10;
  return config;
}

GpioValidationResult validate_esp32_gpio(
    const PhysicalControllerConfig& config) {
  GpioValidationResult result;
  const std::array<int, 13> pins = {
      config.azimuth.driver.step_pin,
      config.azimuth.driver.direction_pin,
      config.azimuth.driver.enable_pin,
      config.azimuth.driver.uart_tx_pin,
      config.azimuth.driver.uart_rx_pin,
      config.azimuth.axis.home_switch_pin,
      config.elevation.driver.step_pin,
      config.elevation.driver.direction_pin,
      config.elevation.driver.enable_pin,
      config.elevation.driver.uart_tx_pin,
      config.elevation.driver.uart_rx_pin,
      config.elevation.axis.home_switch_pin,
      config.emergency_stop_pin,
  };
  for (std::size_t index = 0; index < pins.size(); ++index) {
    if (index == pins.size() - 1 && pins[index] < 0) {
      continue;
    }
    if (!valid_gpio(pins[index])) {
      result.invalid_output_pin = pins[index];
      return result;
    }
    if (bootstrapping_gpio(pins[index])) {
      result.bootstrapping_pin_mask |= 1ULL << pins[index];
    }
    for (std::size_t other = index + 1; other < pins.size(); ++other) {
      if (other == pins.size() - 1 && pins[other] < 0) {
        continue;
      }
      if (pins[index] == pins[other]) {
        result.duplicate_pin = pins[index];
        return result;
      }
    }
  }
  const std::array<int, 8> output_pins = {
      config.azimuth.driver.step_pin,
      config.azimuth.driver.direction_pin,
      config.azimuth.driver.enable_pin,
      config.azimuth.driver.uart_tx_pin,
      config.elevation.driver.step_pin,
      config.elevation.driver.direction_pin,
      config.elevation.driver.enable_pin,
      config.elevation.driver.uart_tx_pin,
  };
  for (const int pin : output_pins) {
    if (input_only_gpio(pin)) {
      result.invalid_output_pin = pin;
      return result;
    }
  }
  result.valid = config.board_name != nullptr && config.board_name[0] != '\0' &&
                 config.emergency_stop_debounce_ms > 0 &&
                 config.azimuth.axis.motion.valid() &&
                 config.elevation.axis.motion.valid();
  return result;
}

}  // namespace radiance3d
