#include "motion_controller.hpp"

#include <cmath>
#include <cstdlib>
#include <utility>

namespace radiance3d {
namespace {

bool finite_positive(const double value) { return std::isfinite(value) && value > 0.0; }

bool angle_in_range(const double value, const AxisConfig& config) {
  return std::isfinite(value) && value >= config.minimum_angle_deg &&
         value <= config.maximum_angle_deg;
}

}  // namespace

double AxisConfig::steps_per_output_revolution() const {
  return static_cast<double>(motor_full_steps_per_revolution) *
         static_cast<double>(microsteps) * gear_ratio;
}

double AxisConfig::commanded_step_angle_deg() const {
  const double steps = steps_per_output_revolution();
  return steps > 0.0 ? 360.0 / steps : 0.0;
}

bool AxisConfig::valid() const {
  return motor_full_steps_per_revolution > 0 && microsteps > 0 &&
         finite_positive(gear_ratio) && std::isfinite(home_offset_deg) &&
         std::isfinite(minimum_angle_deg) && std::isfinite(maximum_angle_deg) &&
         minimum_angle_deg < maximum_angle_deg &&
         home_offset_deg >= minimum_angle_deg && home_offset_deg <= maximum_angle_deg &&
         hold_current_percent <= 100 && settling_time_ms > 0 &&
         motion_timeout_ms > 0 && maximum_bench_test_steps > 0 &&
         finite_positive(maximum_speed_deg_per_s) &&
         finite_positive(acceleration_deg_per_s2) && homing.debounce_ms > 0 &&
         finite_positive(homing.speed_deg_per_s) && finite_positive(homing.backoff_deg) &&
         finite_positive(homing.slow_approach_deg_per_s) &&
         homing.timeout_ms > 0;
}

bool ControllerConfig::valid() const {
  return azimuth.valid() && elevation.valid() && motion_timeout_ms > 0;
}

ControllerConfig provisional_simulator_config() {
  ControllerConfig config;
  config.azimuth.minimum_angle_deg = 0.0;
  config.azimuth.maximum_angle_deg = 360.0;
  config.azimuth.home_offset_deg = 0.0;
  config.elevation.minimum_angle_deg = -90.0;
  config.elevation.maximum_angle_deg = 90.0;
  config.elevation.home_offset_deg = 0.0;
  return config;
}

SimulatedMotionController::SimulatedMotionController(ControllerConfig config)
    : config_(std::move(config)) {
  if (!config_.valid()) {
    state_.fault = FaultCode::invalid_configuration;
  }
}

bool SimulatedMotionController::initialize() { return config_.valid(); }

void SimulatedMotionController::service() {}

const ControllerConfig& SimulatedMotionController::config() const { return config_; }

const ControllerState& SimulatedMotionController::state() const { return state_; }

MotionResult SimulatedMotionController::fail(const FaultCode code) {
  state_.fault = code;
  return MotionResult{false, code};
}

MotionResult SimulatedMotionController::succeed() {
  state_.fault = FaultCode::none;
  return MotionResult{true, FaultCode::none};
}

void SimulatedMotionController::invalidate(AxisState& axis,
                                           const TrustLossReason reason) {
  axis.position_trusted = false;
  axis.homed = false;
  axis.trust_loss_reason = reason;
}

MotionResult SimulatedMotionController::home(const AxisSelection axis,
                                             const std::uint32_t command_id) {
  if (!config_.valid()) {
    return fail(FaultCode::invalid_configuration);
  }
  if (state_.emergency_stop_active) {
    return fail(FaultCode::emergency_stop);
  }
  if (state_.stopped) {
    return fail(FaultCode::stopped);
  }
  if (axis == AxisSelection::azimuth || axis == AxisSelection::both) {
    state_.azimuth.enabled = true;
    state_.azimuth.commanded_position_deg = config_.azimuth.home_offset_deg;
    state_.azimuth.internal_step_position = 0;
    state_.azimuth.target_step_position = 0;
    state_.azimuth.homed = true;
    state_.azimuth.position_trusted = true;
    state_.azimuth.trust_loss_reason = TrustLossReason::none;
    state_.azimuth.last_completed_command = command_id;
  }
  if (axis == AxisSelection::elevation || axis == AxisSelection::both) {
    state_.elevation.enabled = true;
    state_.elevation.commanded_position_deg = config_.elevation.home_offset_deg;
    state_.elevation.internal_step_position = 0;
    state_.elevation.target_step_position = 0;
    state_.elevation.homed = true;
    state_.elevation.position_trusted = true;
    state_.elevation.trust_loss_reason = TrustLossReason::none;
    state_.elevation.last_completed_command = command_id;
  }
  return succeed();
}

MotionResult SimulatedMotionController::move_absolute(const double azimuth_deg,
                                                      const double elevation_deg,
                                                      const double speed_deg_per_s,
                                                      const std::uint32_t command_id) {
  if (!config_.valid()) {
    return fail(FaultCode::invalid_configuration);
  }
  if (state_.emergency_stop_active) {
    return fail(FaultCode::emergency_stop);
  }
  if (state_.stopped) {
    return fail(FaultCode::stopped);
  }
  if (!state_.azimuth.homed || !state_.elevation.homed) {
    return fail(FaultCode::not_homed);
  }
  if (!state_.azimuth.position_trusted || !state_.elevation.position_trusted) {
    return fail(FaultCode::position_untrusted);
  }
  if (!state_.azimuth.enabled || !state_.elevation.enabled) {
    return fail(FaultCode::driver_disabled);
  }
  if (!finite_positive(speed_deg_per_s) ||
      speed_deg_per_s > config_.azimuth.maximum_speed_deg_per_s ||
      speed_deg_per_s > config_.elevation.maximum_speed_deg_per_s) {
    return fail(FaultCode::invalid_argument);
  }
  if (!angle_in_range(azimuth_deg, config_.azimuth) ||
      !angle_in_range(elevation_deg, config_.elevation)) {
    return fail(FaultCode::limit_reached);
  }

  state_.azimuth.commanded_position_deg = azimuth_deg;
  state_.elevation.commanded_position_deg = elevation_deg;
  state_.azimuth.last_completed_command = command_id;
  state_.elevation.last_completed_command = command_id;
  return succeed();
}

MotionResult SimulatedMotionController::stop() {
  state_.stopped = true;
  invalidate(state_.azimuth, TrustLossReason::stopped);
  invalidate(state_.elevation, TrustLossReason::stopped);
  return fail(FaultCode::stopped);
}

MotionResult SimulatedMotionController::emergency_stop() {
  state_.emergency_stop_active = true;
  state_.stopped = true;
  state_.azimuth.enabled = false;
  state_.elevation.enabled = false;
  invalidate(state_.azimuth, TrustLossReason::emergency_stop);
  invalidate(state_.elevation, TrustLossReason::emergency_stop);
  return fail(FaultCode::emergency_stop);
}

MotionResult SimulatedMotionController::clear_fault() {
  if (state_.emergency_stop_active) {
    return fail(FaultCode::emergency_stop);
  }
  state_.stopped = false;
  return succeed();
}

MotionResult SimulatedMotionController::set_enabled(const bool enabled) {
  state_.azimuth.enabled = enabled;
  state_.elevation.enabled = enabled;
  if (!enabled) {
    invalidate(state_.azimuth, TrustLossReason::driver_disabled_during_motion);
    invalidate(state_.elevation, TrustLossReason::driver_disabled_during_motion);
    return fail(FaultCode::driver_disabled);
  }
  return succeed();
}

MotionResult SimulatedMotionController::move_relative(
    const AxisSelection axis, const double delta_deg,
    const double speed_deg_per_s, const std::uint32_t command_id) {
  const double azimuth =
      state_.azimuth.commanded_position_deg +
      ((axis == AxisSelection::azimuth || axis == AxisSelection::both)
           ? delta_deg
           : 0.0);
  const double elevation =
      state_.elevation.commanded_position_deg +
      ((axis == AxisSelection::elevation || axis == AxisSelection::both)
           ? delta_deg
           : 0.0);
  const MotionResult result =
      move_absolute(azimuth, elevation, speed_deg_per_s, command_id);
  if (result.ok) {
    if (axis == AxisSelection::azimuth || axis == AxisSelection::both) {
      state_.azimuth.last_completed_command = command_id;
    }
    if (axis == AxisSelection::elevation || axis == AxisSelection::both) {
      state_.elevation.last_completed_command = command_id;
    }
  }
  return result;
}

MotionResult SimulatedMotionController::bench_move_steps(
    const AxisSelection axis, const std::int64_t signed_steps,
    const std::uint32_t command_id) {
  if (axis == AxisSelection::both || signed_steps == 0) {
    return fail(FaultCode::invalid_argument);
  }
  AxisState& selected =
      axis == AxisSelection::azimuth ? state_.azimuth : state_.elevation;
  const AxisConfig& selected_config =
      axis == AxisSelection::azimuth ? config_.azimuth : config_.elevation;
  if (std::llabs(signed_steps) > selected_config.maximum_bench_test_steps) {
    return fail(FaultCode::limit_reached);
  }
  selected.enabled = true;
  selected.internal_step_position += signed_steps;
  selected.target_step_position = selected.internal_step_position;
  selected.commanded_position_deg =
      static_cast<double>(selected.internal_step_position) * 360.0 /
      selected_config.steps_per_output_revolution();
  selected.last_completed_command = command_id;
  return succeed();
}

MotionResult SimulatedMotionController::stop_axis(
    const AxisSelection axis) {
  if (axis == AxisSelection::azimuth || axis == AxisSelection::both) {
    invalidate(state_.azimuth, TrustLossReason::stopped);
  }
  if (axis == AxisSelection::elevation || axis == AxisSelection::both) {
    invalidate(state_.elevation, TrustLossReason::stopped);
  }
  return succeed();
}

MotionResult SimulatedMotionController::set_axis_enabled(
    const AxisSelection axis, const bool enabled) {
  if (axis == AxisSelection::azimuth || axis == AxisSelection::both) {
    state_.azimuth.enabled = enabled;
    if (!enabled) {
      invalidate(state_.azimuth,
                 TrustLossReason::driver_disabled_during_motion);
    }
  }
  if (axis == AxisSelection::elevation || axis == AxisSelection::both) {
    state_.elevation.enabled = enabled;
    if (!enabled) {
      invalidate(state_.elevation,
                 TrustLossReason::driver_disabled_during_motion);
    }
  }
  return succeed();
}

MotionResult SimulatedMotionController::set_axis_current(
    const AxisSelection axis, const std::uint16_t rms_current_ma) {
  if (axis == AxisSelection::both || rms_current_ma == 0) {
    return fail(FaultCode::invalid_argument);
  }
  AxisConfig& selected =
      axis == AxisSelection::azimuth ? config_.azimuth : config_.elevation;
  selected.motor_rms_current_ma = rms_current_ma;
  return succeed();
}

MotionResult SimulatedMotionController::set_axis_microsteps(
    const AxisSelection axis, const std::uint16_t microsteps) {
  const bool supported =
      microsteps == 1 || microsteps == 2 || microsteps == 4 ||
      microsteps == 8 || microsteps == 16 || microsteps == 32 ||
      microsteps == 64 || microsteps == 128 || microsteps == 256;
  if (axis == AxisSelection::both || !supported) {
    return fail(FaultCode::invalid_argument);
  }
  AxisConfig& selected =
      axis == AxisSelection::azimuth ? config_.azimuth : config_.elevation;
  AxisState& selected_state =
      axis == AxisSelection::azimuth ? state_.azimuth : state_.elevation;
  if (selected.microsteps != microsteps) {
    selected.microsteps = microsteps;
    invalidate(selected_state, TrustLossReason::configuration_changed);
  }
  return succeed();
}

DriverCapabilities SimulatedMotionController::driver_capabilities(
    AxisSelection) const {
  return DriverCapabilities{true, true, true, true, true, true};
}

DriverStatus SimulatedMotionController::driver_status(
    const AxisSelection axis) const {
  if (axis == AxisSelection::elevation) {
    return state_.elevation.last_driver_status;
  }
  return state_.azimuth.last_driver_status;
}

void SimulatedMotionController::report_fault(const FaultCode code) { state_.fault = code; }

}  // namespace radiance3d
