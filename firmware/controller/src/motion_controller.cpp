#include "motion_controller.hpp"

#include <cmath>
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
         finite_positive(maximum_speed_deg_per_s) &&
         finite_positive(acceleration_deg_per_s2) && homing.debounce_ms > 0 &&
         finite_positive(homing.speed_deg_per_s) && finite_positive(homing.backoff_deg) &&
         finite_positive(homing.slow_approach_deg_per_s);
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

void SimulatedMotionController::invalidate(AxisState& axis) {
  axis.position_trusted = false;
  axis.homed = false;
}

MotionResult SimulatedMotionController::home(const AxisSelection axis) {
  if (!config_.valid()) {
    return fail(FaultCode::invalid_configuration);
  }
  if (state_.emergency_stop_active) {
    return fail(FaultCode::emergency_stop);
  }
  if (state_.stopped) {
    return fail(FaultCode::stopped);
  }
  if (!state_.azimuth.enabled || !state_.elevation.enabled) {
    return fail(FaultCode::driver_disabled);
  }
  if (axis == AxisSelection::azimuth || axis == AxisSelection::both) {
    state_.azimuth.commanded_position_deg = config_.azimuth.home_offset_deg;
    state_.azimuth.homed = true;
    state_.azimuth.position_trusted = true;
  }
  if (axis == AxisSelection::elevation || axis == AxisSelection::both) {
    state_.elevation.commanded_position_deg = config_.elevation.home_offset_deg;
    state_.elevation.homed = true;
    state_.elevation.position_trusted = true;
  }
  return succeed();
}

MotionResult SimulatedMotionController::move_absolute(const double azimuth_deg,
                                                      const double elevation_deg,
                                                      const double speed_deg_per_s) {
  if (!config_.valid()) {
    return fail(FaultCode::invalid_configuration);
  }
  if (state_.emergency_stop_active) {
    return fail(FaultCode::emergency_stop);
  }
  if (state_.stopped) {
    return fail(FaultCode::stopped);
  }
  if (!state_.azimuth.enabled || !state_.elevation.enabled) {
    return fail(FaultCode::driver_disabled);
  }
  if (!state_.azimuth.homed || !state_.elevation.homed) {
    return fail(FaultCode::not_homed);
  }
  if (!state_.azimuth.position_trusted || !state_.elevation.position_trusted) {
    return fail(FaultCode::position_untrusted);
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
  return succeed();
}

MotionResult SimulatedMotionController::stop() {
  state_.stopped = true;
  invalidate(state_.azimuth);
  invalidate(state_.elevation);
  return fail(FaultCode::stopped);
}

MotionResult SimulatedMotionController::emergency_stop() {
  state_.emergency_stop_active = true;
  state_.stopped = true;
  invalidate(state_.azimuth);
  invalidate(state_.elevation);
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
    invalidate(state_.azimuth);
    invalidate(state_.elevation);
    return fail(FaultCode::driver_disabled);
  }
  return succeed();
}

void SimulatedMotionController::report_fault(const FaultCode code) { state_.fault = code; }

}  // namespace radiance3d
