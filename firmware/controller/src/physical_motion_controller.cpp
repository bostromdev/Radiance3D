#include "physical_motion_controller.hpp"

#include <utility>

namespace radiance3d {
namespace {

bool elapsed(const std::uint64_t now, const std::uint64_t started,
             const std::uint64_t duration) {
  return now - started >= duration;
}

}  // namespace

PhysicalMotionController::PhysicalMotionController(
    HardwarePlatform& platform, AxisController& azimuth,
    AxisController& elevation, PhysicalControllerConfig physical_config)
    : platform_(platform),
      azimuth_(azimuth),
      elevation_(elevation),
      physical_config_(std::move(physical_config)) {
  config_.azimuth = physical_config_.azimuth.axis.motion;
  config_.elevation = physical_config_.elevation.axis.motion;
  config_.motion_timeout_ms =
      config_.azimuth.motion_timeout_ms > config_.elevation.motion_timeout_ms
          ? config_.azimuth.motion_timeout_ms
          : config_.elevation.motion_timeout_ms;
  config_.emergency_stop_active_low =
      physical_config_.emergency_stop_active_low;
}

const PhysicalControllerConfig&
PhysicalMotionController::physical_config() const {
  return physical_config_;
}

const ControllerConfig& PhysicalMotionController::config() const {
  return config_;
}

const ControllerState& PhysicalMotionController::state() const {
  return state_;
}

AxisController* PhysicalMotionController::selected_axis(
    const AxisSelection axis) {
  if (axis == AxisSelection::azimuth) {
    return &azimuth_;
  }
  if (axis == AxisSelection::elevation) {
    return &elevation_;
  }
  return nullptr;
}

const AxisController* PhysicalMotionController::selected_axis(
    const AxisSelection axis) const {
  if (axis == AxisSelection::azimuth) {
    return &azimuth_;
  }
  if (axis == AxisSelection::elevation) {
    return &elevation_;
  }
  return nullptr;
}

bool PhysicalMotionController::emergency_input_active_raw() const {
  if (physical_config_.emergency_stop_pin < 0) {
    return false;
  }
  const bool level_high =
      platform_.read_pin(physical_config_.emergency_stop_pin);
  return physical_config_.emergency_stop_active_low ? !level_high
                                                    : level_high;
}

void PhysicalMotionController::update_emergency_input(
    const std::uint64_t now_us) {
  const bool raw = emergency_input_active_raw();
  if (!emergency_input_initialized_) {
    emergency_candidate_ = raw;
    emergency_stable_ = raw;
    emergency_changed_us_ = now_us;
    emergency_input_initialized_ = true;
    return;
  }
  if (raw != emergency_candidate_) {
    emergency_candidate_ = raw;
    emergency_changed_us_ = now_us;
  }
  const std::uint64_t debounce_us =
      static_cast<std::uint64_t>(
          physical_config_.emergency_stop_debounce_ms) *
      1000ULL;
  if (emergency_candidate_ != emergency_stable_ &&
      elapsed(now_us, emergency_changed_us_, debounce_us)) {
    emergency_stable_ = emergency_candidate_;
  }
}

void PhysicalMotionController::synchronize_state() {
  state_.azimuth = azimuth_.state();
  state_.elevation = elevation_.state();
  state_.emergency_stop_active = emergency_latched_;
  if (emergency_latched_) {
    state_.fault = FaultCode::emergency_stop;
  } else {
    const FaultCode axis_fault = active_axis_fault();
    if (axis_fault != FaultCode::none) {
      state_.fault = axis_fault;
    }
  }
  state_.stopped = emergency_latched_ ||
                   (!state_.azimuth.moving && !state_.elevation.moving &&
                    state_.fault == FaultCode::stopped);
}

FaultCode PhysicalMotionController::active_axis_fault() const {
  // Emergency stop is handled by synchronize_state() first.  Prefer an
  // azimuth fault only when both occur in the same service cycle; either is
  // promoted to the controller so protocol events cannot claim completion.
  if (azimuth_.state().fault != FaultCode::none) {
    return azimuth_.state().fault;
  }
  return elevation_.state().fault;
}

bool PhysicalMotionController::initialize() {
  state_ = ControllerState{};
  const GpioValidationResult gpio =
      validate_esp32_gpio(physical_config_);
  if (!gpio.valid) {
    state_.fault = FaultCode::invalid_configuration;
    return false;
  }
  if (physical_config_.emergency_stop_pin >= 0 &&
      !platform_.configure_pin(physical_config_.emergency_stop_pin,
                               physical_config_.emergency_stop_input_mode)) {
    state_.fault = FaultCode::invalid_configuration;
    return false;
  }
  update_emergency_input(platform_.monotonic_micros());
  const bool azimuth_ok = azimuth_.initialize();
  const bool elevation_ok = elevation_.initialize();
  synchronize_state();
  initialized_ = azimuth_ok && elevation_ok;
  if (!initialized_) {
    state_.fault = FaultCode::driver_communication;
  }
  if (emergency_stable_) {
    emergency_stop();
    return false;
  }
  return initialized_;
}

MotionResult PhysicalMotionController::reject(const FaultCode fault) {
  state_.fault = fault;
  return MotionResult{false, fault};
}

void PhysicalMotionController::service() {
  const std::uint64_t now_us = platform_.monotonic_micros();
  update_emergency_input(now_us);
  if (emergency_stable_ && !emergency_latched_) {
    emergency_stop();
    return;
  }
  azimuth_.service();
  elevation_.service();

  if (coordinated_move_active_) {
    const bool azimuth_fault =
        azimuth_.state().fault == FaultCode::driver_critical ||
        azimuth_.state().fault == FaultCode::driver_communication ||
        azimuth_.state().fault == FaultCode::motion_timeout ||
        azimuth_.state().fault == FaultCode::unexpected_home_switch;
    const bool elevation_fault =
        elevation_.state().fault == FaultCode::driver_critical ||
        elevation_.state().fault == FaultCode::driver_communication ||
        elevation_.state().fault == FaultCode::motion_timeout ||
        elevation_.state().fault == FaultCode::unexpected_home_switch;
    if (azimuth_fault || elevation_fault) {
      azimuth_.stop(true);
      elevation_.stop(true);
      state_.fault = FaultCode::driver_critical;
      coordinated_move_active_ = false;
    } else if (!azimuth_.state().moving && !elevation_.state().moving) {
      coordinated_move_active_ = false;
      state_.fault = FaultCode::none;
    }
  }
  synchronize_state();
}

void PhysicalMotionController::service_diagnostics() {
  if (!initialized_ || emergency_latched_ || azimuth_.state().moving ||
      elevation_.state().moving) {
    return;
  }
  azimuth_.service_diagnostics();
  elevation_.service_diagnostics();
  synchronize_state();
}

MotionResult PhysicalMotionController::home(
    const AxisSelection axis, const std::uint32_t command_id) {
  if (!initialized_ || emergency_latched_) {
    return reject(emergency_latched_ ? FaultCode::emergency_stop
                                     : FaultCode::invalid_configuration);
  }
  if (axis == AxisSelection::both) {
    const MotionResult azimuth_result =
        azimuth_.start_homing(command_id);
    if (!azimuth_result.ok) {
      return reject(azimuth_result.fault);
    }
    const MotionResult elevation_result =
        elevation_.start_homing(command_id);
    if (!elevation_result.ok) {
      azimuth_.stop(true);
      return reject(elevation_result.fault);
    }
    coordinated_move_active_ = true;
    synchronize_state();
    return MotionResult{true, FaultCode::none};
  }
  AxisController* selected = selected_axis(axis);
  if (selected == nullptr) {
    return reject(FaultCode::invalid_argument);
  }
  const MotionResult result = selected->start_homing(command_id);
  synchronize_state();
  return result;
}

MotionResult PhysicalMotionController::move_absolute(
    const double azimuth_deg, const double elevation_deg,
    const double speed_deg_per_s, const std::uint32_t command_id) {
  if (!initialized_ || emergency_latched_) {
    return reject(emergency_latched_ ? FaultCode::emergency_stop
                                     : FaultCode::invalid_configuration);
  }
  const MotionResult azimuth_result = azimuth_.move_absolute_degrees(
      azimuth_deg, speed_deg_per_s, command_id);
  if (!azimuth_result.ok) {
    return reject(azimuth_result.fault);
  }
  const MotionResult elevation_result = elevation_.move_absolute_degrees(
      elevation_deg, speed_deg_per_s, command_id);
  if (!elevation_result.ok) {
    azimuth_.stop(true);
    return reject(elevation_result.fault);
  }
  coordinated_move_active_ =
      azimuth_.state().moving || elevation_.state().moving;
  synchronize_state();
  return MotionResult{true, FaultCode::none};
}

MotionResult PhysicalMotionController::move_relative(
    const AxisSelection axis, const double delta_deg,
    const double speed_deg_per_s, const std::uint32_t command_id) {
  if (!initialized_ || emergency_latched_) {
    return reject(emergency_latched_ ? FaultCode::emergency_stop
                                     : FaultCode::invalid_configuration);
  }
  if (axis == AxisSelection::both) {
    const MotionResult azimuth_result = azimuth_.move_relative_degrees(
        delta_deg, speed_deg_per_s, command_id);
    if (!azimuth_result.ok) {
      return reject(azimuth_result.fault);
    }
    const MotionResult elevation_result = elevation_.move_relative_degrees(
        delta_deg, speed_deg_per_s, command_id);
    if (!elevation_result.ok) {
      azimuth_.stop(true);
      return reject(elevation_result.fault);
    }
    coordinated_move_active_ = true;
    synchronize_state();
    return MotionResult{true, FaultCode::none};
  }
  AxisController* selected = selected_axis(axis);
  if (selected == nullptr) {
    return reject(FaultCode::invalid_argument);
  }
  const MotionResult result = selected->move_relative_degrees(
      delta_deg, speed_deg_per_s, command_id);
  synchronize_state();
  return result;
}

MotionResult PhysicalMotionController::bench_move_steps(
    const AxisSelection axis, const std::int64_t signed_steps,
    const std::uint32_t command_id) {
  if (!initialized_ || emergency_latched_) {
    return reject(emergency_latched_ ? FaultCode::emergency_stop
                                     : FaultCode::invalid_configuration);
  }
  AxisController* selected = selected_axis(axis);
  if (selected == nullptr) {
    return reject(FaultCode::invalid_argument);
  }
  const MotionResult result =
      selected->bench_move_steps(signed_steps, command_id);
  synchronize_state();
  return result;
}

MotionResult PhysicalMotionController::stop_axis(
    const AxisSelection axis) {
  if (axis == AxisSelection::both) {
    return stop();
  }
  AxisController* selected = selected_axis(axis);
  if (selected == nullptr) {
    return reject(FaultCode::invalid_argument);
  }
  const MotionResult result = selected->stop(true);
  coordinated_move_active_ = false;
  synchronize_state();
  return result;
}

MotionResult PhysicalMotionController::stop() {
  azimuth_.stop(true);
  elevation_.stop(true);
  coordinated_move_active_ = false;
  state_.fault = FaultCode::stopped;
  synchronize_state();
  return MotionResult{true, FaultCode::stopped};
}

MotionResult PhysicalMotionController::emergency_stop() {
  emergency_latched_ = true;
  coordinated_move_active_ = false;
  azimuth_.emergency_stop();
  elevation_.emergency_stop();
  state_.fault = FaultCode::emergency_stop;
  synchronize_state();
  return MotionResult{true, FaultCode::emergency_stop};
}

MotionResult PhysicalMotionController::clear_fault() {
  update_emergency_input(platform_.monotonic_micros());
  if (emergency_stable_) {
    return reject(FaultCode::emergency_stop);
  }
  emergency_latched_ = false;
  const MotionResult azimuth_result = azimuth_.clear_fault();
  const MotionResult elevation_result = elevation_.clear_fault();
  if (!azimuth_result.ok || !elevation_result.ok) {
    return reject(!azimuth_result.ok ? azimuth_result.fault
                                     : elevation_result.fault);
  }
  state_.fault = FaultCode::none;
  synchronize_state();
  return MotionResult{true, FaultCode::none};
}

MotionResult PhysicalMotionController::set_enabled(const bool enabled) {
  if (!initialized_ || emergency_latched_) {
    return reject(emergency_latched_ ? FaultCode::emergency_stop
                                     : FaultCode::invalid_configuration);
  }
  const MotionResult azimuth_result = azimuth_.set_enabled(enabled);
  const MotionResult elevation_result = elevation_.set_enabled(enabled);
  if (!azimuth_result.ok || !elevation_result.ok) {
    return reject(!azimuth_result.ok ? azimuth_result.fault
                                     : elevation_result.fault);
  }
  state_.fault = enabled ? FaultCode::none : FaultCode::driver_disabled;
  synchronize_state();
  return MotionResult{true, enabled ? FaultCode::none
                                   : FaultCode::driver_disabled};
}

MotionResult PhysicalMotionController::set_axis_enabled(
    const AxisSelection axis, const bool enabled) {
  if (!initialized_ || emergency_latched_) {
    return reject(emergency_latched_ ? FaultCode::emergency_stop
                                     : FaultCode::invalid_configuration);
  }
  if (axis == AxisSelection::both) {
    return set_enabled(enabled);
  }
  AxisController* selected = selected_axis(axis);
  if (selected == nullptr) {
    return reject(FaultCode::invalid_argument);
  }
  const MotionResult result = selected->set_enabled(enabled);
  if (result.ok) {
    state_.fault = enabled ? FaultCode::none : FaultCode::driver_disabled;
  }
  synchronize_state();
  return result;
}

MotionResult PhysicalMotionController::set_axis_current(
    const AxisSelection axis, const std::uint16_t rms_current_ma) {
  if (!initialized_ || emergency_latched_) {
    return reject(emergency_latched_ ? FaultCode::emergency_stop
                                     : FaultCode::invalid_configuration);
  }
  AxisController* selected = selected_axis(axis);
  if (selected == nullptr) {
    return reject(FaultCode::invalid_argument);
  }
  const MotionResult result = selected->set_current(rms_current_ma);
  if (result.ok) {
    if (axis == AxisSelection::azimuth) {
      config_.azimuth = azimuth_.config().motion;
      physical_config_.azimuth.axis.motion = config_.azimuth;
    } else {
      config_.elevation = elevation_.config().motion;
      physical_config_.elevation.axis.motion = config_.elevation;
    }
    state_.fault = FaultCode::none;
  }
  synchronize_state();
  return result;
}

MotionResult PhysicalMotionController::set_axis_microsteps(
    const AxisSelection axis, const std::uint16_t microsteps) {
  if (!initialized_ || emergency_latched_) {
    return reject(emergency_latched_ ? FaultCode::emergency_stop
                                     : FaultCode::invalid_configuration);
  }
  AxisController* selected = selected_axis(axis);
  if (selected == nullptr) {
    return reject(FaultCode::invalid_argument);
  }
  const MotionResult result = selected->set_microsteps(microsteps);
  if (result.ok) {
    if (axis == AxisSelection::azimuth) {
      config_.azimuth = azimuth_.config().motion;
      physical_config_.azimuth.axis.motion = config_.azimuth;
    } else {
      config_.elevation = elevation_.config().motion;
      physical_config_.elevation.axis.motion = config_.elevation;
    }
    state_.fault = FaultCode::none;
  }
  synchronize_state();
  return result;
}

DriverCapabilities PhysicalMotionController::driver_capabilities(
    const AxisSelection axis) const {
  const AxisController* selected = selected_axis(axis);
  return selected == nullptr ? DriverCapabilities{}
                             : selected->driver_capabilities();
}

DriverStatus PhysicalMotionController::driver_status(
    const AxisSelection axis) const {
  const AxisController* selected = selected_axis(axis);
  return selected == nullptr ? DriverStatus{} : selected->driver_status();
}

void PhysicalMotionController::report_fault(const FaultCode code) {
  state_.fault = code;
}

}  // namespace radiance3d
