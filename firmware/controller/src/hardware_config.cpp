#include "hardware_config.hpp"

#include "hardware_profile_generated.hpp"

#include <array>
#include <cstddef>

namespace radiance3d {
namespace {

bool valid_gpio(const int pin) { return pin >= 0 && pin <= 39; }

bool input_only_gpio(const int pin) { return pin >= 34 && pin <= 39; }

bool bootstrapping_gpio(const int pin) {
  return pin == 0 || pin == 2 || pin == 5 || pin == 12 || pin == 15;
}

PinMode input_mode(const bool pullup, const bool pulldown) {
  if (pullup) {
    return PinMode::input_pullup;
  }
  if (pulldown) {
    return PinMode::input_pulldown;
  }
  return PinMode::input;
}

AxisConfig motion_from_profile(
    const generated_profile::AxisProfile& profile) {
  AxisConfig config;
  config.motor_full_steps_per_revolution =
      profile.motor_full_steps_per_revolution;
  config.microsteps = profile.microsteps;
  config.motor_rms_current_ma = profile.commissioning_current_ma;
  config.hold_current_percent = profile.hold_current_percent;
  config.gear_ratio = {profile.gear_ratio_numerator,
                       profile.gear_ratio_denominator};
  config.direction_inverted = profile.direction_inverted;
  config.home_offset_deg = profile.home_offset_deg;
  config.minimum_angle_deg = profile.minimum_angle_deg;
  config.maximum_angle_deg = profile.maximum_angle_deg;
  config.maximum_speed_deg_per_s = profile.maximum_speed_deg_per_s;
  config.acceleration_deg_per_s2 = profile.acceleration_deg_per_s2;
  config.settling_time_ms = profile.settling_time_ms;
  config.motion_timeout_ms = profile.motion_timeout_ms;
  config.maximum_bench_test_steps = profile.maximum_bench_test_steps;
  config.homing.switch_normally_closed = profile.home_switch_normally_closed;
  config.homing.direction_negative = profile.homing_direction_negative;
  config.homing.debounce_ms = profile.home_switch_debounce_ms;
  config.homing.speed_deg_per_s = profile.home_speed_deg_per_s;
  config.homing.slow_approach_deg_per_s = profile.slow_home_speed_deg_per_s;
  config.homing.backoff_deg = profile.homing_backoff_deg;
  config.homing.timeout_ms = profile.motion_timeout_ms;
  return config;
}

PhysicalAxisDefinition axis_from_profile(
    const generated_profile::AxisProfile& profile) {
  PhysicalAxisDefinition definition;
  definition.axis.name = profile.name;
  definition.axis.motion = motion_from_profile(profile);
  definition.axis.home_switch_pin = profile.home_switch_pin;
  definition.axis.home_switch_input_mode =
      input_mode(profile.home_switch_pullup, profile.home_switch_pulldown);
  definition.driver.uart_channel = profile.uart_channel;
  definition.driver.address = profile.uart_address;
  definition.driver.uart_tx_pin = profile.uart_tx_pin;
  definition.driver.uart_rx_pin = profile.uart_rx_pin;
  definition.driver.step_pin = profile.step_pin;
  definition.driver.direction_pin = profile.direction_pin;
  definition.driver.enable_pin = profile.enable_pin;
  definition.driver.enable_active_low = generated_profile::kTmcEnableActiveLow;
  definition.driver.direction_inverted = profile.direction_inverted;
  definition.driver.sense_resistor_milliohms =
      generated_profile::kTmcSenseResistorMilliohms;
  definition.driver.maximum_rms_current_ma = profile.maximum_rms_current_ma;
  definition.driver.uart_baud = generated_profile::kTmcUartBaud;
  definition.driver.uart_timeout_ms = generated_profile::kTmcUartTimeoutMs;
  definition.driver.uart_single_wire =
      generated_profile::kTmcSingleWirePdnUart;
  definition.driver.write_echo_expected =
      generated_profile::kTmcWriteEchoExpected;
  return definition;
}

}  // namespace

PhysicalControllerConfig provisional_esp32_dev_config() {
  PhysicalControllerConfig config;
  config.board_name = generated_profile::kBoardName;
  config.protocol_version = generated_profile::kProtocolVersion;
  config.azimuth = axis_from_profile(generated_profile::kAzimuth);
  config.elevation = axis_from_profile(generated_profile::kElevation);
  config.emergency_stop_pin = generated_profile::kEmergencyStopPin;
  config.emergency_stop_active_low = generated_profile::kEmergencyStopActiveLow;
  config.emergency_stop_input_mode = input_mode(
      generated_profile::kEmergencyStopPullup,
      generated_profile::kEmergencyStopPulldown);
  config.emergency_stop_debounce_ms = generated_profile::kEmergencyStopDebounceMs;
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
