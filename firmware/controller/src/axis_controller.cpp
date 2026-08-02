#include "axis_controller.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>

namespace radiance3d {
namespace {

// GPTimer's documented practical minimum alarm period is 5 us.  This remains
// comfortably above the TMC2209 STEP/DIR minima and avoids sub-period races.
constexpr std::uint64_t kStepPulseWidthUs = 5;
constexpr std::uint64_t kDirectionSetupUs = 5;
constexpr std::uint64_t kMinimumStepLowUs = 5;
constexpr std::uint64_t kDriverStatusIntervalUs = 100000;

bool elapsed(const std::uint64_t now, const std::uint64_t started,
             const std::uint64_t duration) {
  return now - started >= duration;
}

}  // namespace

AxisController::AxisController(HardwarePlatform& platform, StepperDriver& driver,
                               PhysicalAxisConfig config,
                               StepPulseScheduler* pulse_scheduler)
    : platform_(platform),
      driver_(driver),
      config_(config),
      pulse_scheduler_(pulse_scheduler) {}

const PhysicalAxisConfig& AxisController::config() const { return config_; }

const AxisState& AxisController::state() const { return state_; }

AxisState& AxisController::mutable_state() { return state_; }

DriverCapabilities AxisController::driver_capabilities() const {
  return driver_.capabilities();
}

DriverStatus AxisController::driver_status() const {
  return state_.last_driver_status;
}

bool AxisController::degrees_to_steps(const double degrees,
                                      std::int64_t& steps) const {
  if (!std::isfinite(degrees) || !config_.motion.valid()) {
    return false;
  }
  std::int64_t steps_per_revolution = 0;
  if (!config_.motion.steps_per_output_revolution_exact(
          steps_per_revolution)) {
    return false;
  }
  const long double scaled =
      static_cast<long double>(degrees) *
      static_cast<long double>(steps_per_revolution) /
      360.0L;
  if (scaled >
          static_cast<long double>(std::numeric_limits<std::int64_t>::max()) ||
      scaled <
          static_cast<long double>(std::numeric_limits<std::int64_t>::min())) {
    return false;
  }
  steps = static_cast<std::int64_t>(std::llround(scaled));
  return true;
}

double AxisController::steps_to_degrees(const std::int64_t steps) const {
  std::int64_t steps_per_revolution = 0;
  if (!config_.motion.steps_per_output_revolution_exact(
          steps_per_revolution)) {
    return 0.0;
  }
  return static_cast<double>(steps) * 360.0 /
         static_cast<double>(steps_per_revolution);
}

bool AxisController::motor_full_steps_to_output_steps(
    const std::int64_t motor_full_steps, std::int64_t& output_steps) const {
  std::int64_t output_steps_per_full_step = 0;
  if (!config_.motion.output_steps_per_motor_full_step(
          output_steps_per_full_step) ||
      (motor_full_steps > 0 &&
       motor_full_steps > std::numeric_limits<std::int64_t>::max() /
                              output_steps_per_full_step) ||
      (motor_full_steps < 0 &&
       motor_full_steps < std::numeric_limits<std::int64_t>::min() /
                              output_steps_per_full_step)) {
    return false;
  }
  output_steps = motor_full_steps * output_steps_per_full_step;
  return true;
}

std::int64_t AxisController::minimum_steps() const {
  std::int64_t value = 0;
  degrees_to_steps(config_.motion.minimum_angle_deg, value);
  return value;
}

std::int64_t AxisController::maximum_steps() const {
  std::int64_t value = 0;
  degrees_to_steps(config_.motion.maximum_angle_deg, value);
  return value;
}

bool AxisController::initialize() {
  state_ = AxisState{};
  state_.enabled = false;
  state_.trust_loss_reason = TrustLossReason::startup;
  if (config_.name == nullptr || config_.name[0] == '\0' ||
      !config_.motion.valid() ||
      config_.motion.motor_rms_current_ma == 0 ||
      (config_.home_switch_pin >= 0 &&
       !platform_.configure_pin(config_.home_switch_pin,
                                config_.home_switch_input_mode)) ||
      !driver_.initialize()) {
    state_.fault = driver_.is_connected()
                       ? FaultCode::invalid_configuration
                       : FaultCode::driver_communication;
    return false;
  }
  if (!driver_.set_current_milliamps(
          config_.motion.motor_rms_current_ma,
          config_.motion.hold_current_percent) ||
      !driver_.set_microsteps(config_.motion.microsteps) ||
      !driver_.set_interpolation(true) ||
      !driver_.set_chopper_mode(ChopperMode::stealthchop)) {
    driver_.disable();
    state_.fault = FaultCode::invalid_configuration;
    return false;
  }
  if (pulse_scheduler_ != nullptr && !pulse_scheduler_->initialize()) {
    driver_.disable();
    state_.fault = FaultCode::invalid_configuration;
    return false;
  }
  driver_.disable();
  update_home_switch(platform_.monotonic_micros());
  state_.last_driver_status = driver_.read_status();
  if (state_.last_driver_status.critical_fault()) {
    state_.fault = FaultCode::driver_critical;
    return false;
  }
  return true;
}

MotionResult AxisController::fail(const FaultCode fault,
                                  const TrustLossReason reason,
                                  const bool disable_driver) {
  stop_pulse_generation();
  state_.fault = fault;
  state_.homing_phase = HomingPhase::failed;
  mark_position_untrusted(reason);
  if (disable_driver) {
    driver_.disable();
    state_.enabled = false;
  }
  return MotionResult{false, fault};
}

MotionResult AxisController::succeed() {
  state_.fault = FaultCode::none;
  return MotionResult{true, FaultCode::none};
}

void AxisController::mark_position_untrusted(const TrustLossReason reason) {
  state_.position_trusted = false;
  state_.homed = false;
  state_.trust_loss_reason = reason;
}

bool AxisController::home_switch_active_raw() const {
  if (config_.home_switch_pin < 0) {
    return false;
  }
  const bool level_high = platform_.read_pin(config_.home_switch_pin);
  return config_.motion.homing.switch_normally_closed ? level_high
                                                      : !level_high;
}

void AxisController::update_home_switch(const std::uint64_t now_us) {
  const bool raw = home_switch_active_raw();
  if (!home_input_initialized_) {
    raw_home_candidate_ = raw;
    debounced_home_active_ = raw;
    home_candidate_changed_us_ = now_us;
    home_input_initialized_ = true;
    state_.home_switch_active = raw;
    return;
  }
  if (raw != raw_home_candidate_) {
    raw_home_candidate_ = raw;
    home_candidate_changed_us_ = now_us;
  }
  const std::uint64_t debounce_us =
      static_cast<std::uint64_t>(config_.motion.homing.debounce_ms) * 1000ULL;
  if (raw_home_candidate_ != debounced_home_active_ &&
      elapsed(now_us, home_candidate_changed_us_, debounce_us)) {
    debounced_home_active_ = raw_home_candidate_;
  }
  state_.home_switch_active = debounced_home_active_;
}

MotionResult AxisController::start_step_move(
    const std::int64_t target_steps, const double speed_deg_per_s,
    const MotionPurpose purpose, const std::uint32_t command_id,
    const bool require_trust) {
  if (state_.moving || !std::isfinite(speed_deg_per_s) ||
      speed_deg_per_s <= 0.0 ||
      speed_deg_per_s > config_.motion.maximum_speed_deg_per_s) {
    return MotionResult{false, FaultCode::invalid_argument};
  }
  if (require_trust && !state_.position_trusted) {
    return MotionResult{false, FaultCode::not_homed};
  }
  if (purpose == MotionPurpose::normal &&
      (target_steps < minimum_steps() || target_steps > maximum_steps())) {
    return MotionResult{false, FaultCode::limit_reached};
  }
  if (!driver_.is_connected() || state_.last_driver_status.critical_fault()) {
    return fail(FaultCode::driver_critical, TrustLossReason::driver_fault,
                true);
  }
  if (!state_.enabled) {
    if (!driver_.enable()) {
      return fail(FaultCode::driver_communication,
                  TrustLossReason::driver_fault, true);
    }
    state_.enabled = true;
  }
  if (target_steps == state_.internal_step_position) {
    state_.target_step_position = target_steps;
    state_.last_completed_command = command_id;
    return succeed();
  }

  step_direction_ =
      target_steps > state_.internal_step_position ? 1 : -1;
  if (!driver_.set_direction(step_direction_ > 0)) {
    return fail(FaultCode::driver_communication,
                TrustLossReason::driver_fault, true);
  }
  state_.target_step_position = target_steps;
  state_.moving = true;
  motion_purpose_ = purpose;
  motion_started_us_ = platform_.monotonic_micros();
  active_command_id_ = command_id;
  requested_speed_steps_s_ =
      speed_deg_per_s * config_.motion.steps_per_output_revolution() / 360.0;
  const double acceleration_steps_s2 =
      config_.motion.acceleration_deg_per_s2 *
      config_.motion.steps_per_output_revolution() / 360.0;
  current_speed_steps_s_ =
      std::min(requested_speed_steps_s_,
               std::max(1.0, std::sqrt(2.0 * acceleration_steps_s2)));
  if (pulse_scheduler_ != nullptr) {
    if (!pulse_scheduler_->schedule_pulse(kDirectionSetupUs)) {
      return fail(FaultCode::driver_communication,
                  TrustLossReason::driver_fault, true);
    }
  } else {
    next_edge_us_ = motion_started_us_ + kDirectionSetupUs;
  }
  step_high_ = false;
  return succeed();
}

MotionResult AxisController::move_absolute_degrees(
    const double target_deg, const double speed_deg_per_s,
    const std::uint32_t command_id) {
  std::int64_t target_steps = 0;
  if (!degrees_to_steps(target_deg, target_steps)) {
    return MotionResult{false, FaultCode::invalid_argument};
  }
  return start_step_move(target_steps, speed_deg_per_s,
                         MotionPurpose::normal, command_id, true);
}

MotionResult AxisController::move_relative_degrees(
    const double delta_deg, const double speed_deg_per_s,
    const std::uint32_t command_id) {
  if (!state_.position_trusted) {
    return MotionResult{false, FaultCode::not_homed};
  }
  std::int64_t delta_steps = 0;
  if (!degrees_to_steps(delta_deg, delta_steps) ||
      (delta_steps > 0 &&
       state_.internal_step_position >
           std::numeric_limits<std::int64_t>::max() - delta_steps) ||
      (delta_steps < 0 &&
       state_.internal_step_position <
           std::numeric_limits<std::int64_t>::min() - delta_steps)) {
    return MotionResult{false, FaultCode::invalid_argument};
  }
  return start_step_move(state_.internal_step_position + delta_steps,
                         speed_deg_per_s, MotionPurpose::normal, command_id,
                         true);
}

MotionResult AxisController::bench_move_steps(
    const std::int64_t signed_steps, const std::uint32_t command_id) {
  if (signed_steps == 0 ||
      signed_steps > config_.motion.maximum_bench_test_steps ||
      signed_steps < -config_.motion.maximum_bench_test_steps) {
    return MotionResult{false, FaultCode::invalid_argument};
  }
  if ((signed_steps > 0 &&
       state_.internal_step_position >
           std::numeric_limits<std::int64_t>::max() - signed_steps) ||
      (signed_steps < 0 &&
       state_.internal_step_position <
           std::numeric_limits<std::int64_t>::min() - signed_steps)) {
    return MotionResult{false, FaultCode::invalid_argument};
  }
  const double conservative_speed =
      std::min(config_.motion.maximum_speed_deg_per_s,
               config_.motion.homing.speed_deg_per_s);
  return start_step_move(state_.internal_step_position + signed_steps,
                         conservative_speed, MotionPurpose::bench, command_id,
                         false);
}

MotionResult AxisController::start_homing(const std::uint32_t command_id) {
  if (config_.home_switch_pin < 0) {
    return MotionResult{false, FaultCode::invalid_configuration};
  }
  if (state_.moving) {
    return MotionResult{false, FaultCode::invalid_argument};
  }
  update_home_switch(platform_.monotonic_micros());
  mark_position_untrusted(TrustLossReason::homing_failed);
  state_.homing_phase = HomingPhase::validate_switch;
  if (state_.home_switch_active) {
    return fail(FaultCode::homing_stuck_switch,
                TrustLossReason::homing_failed, true);
  }
  homing_started_us_ = platform_.monotonic_micros();
  state_.homing_phase = HomingPhase::fast_approach;
  const std::int64_t travel =
      std::llabs(maximum_steps() - minimum_steps()) +
      std::llabs(maximum_steps() - minimum_steps()) / 10 + 1;
  const std::int64_t target =
      state_.internal_step_position +
      (config_.motion.homing.direction_negative ? -travel : travel);
  return start_step_move(target, config_.motion.homing.speed_deg_per_s,
                         MotionPurpose::homing_fast, command_id, false);
}

std::uint64_t AxisController::step_interval_us(
    const std::int64_t remaining_steps) {
  const double acceleration_steps_s2 =
      config_.motion.acceleration_deg_per_s2 *
      config_.motion.steps_per_output_revolution() / 360.0;
  const double acceleration_limited =
      std::sqrt(current_speed_steps_s_ * current_speed_steps_s_ +
                2.0 * acceleration_steps_s2);
  const double braking_limited =
      std::sqrt(2.0 * acceleration_steps_s2 *
                std::max<std::int64_t>(remaining_steps, 1));
  current_speed_steps_s_ =
      std::max(1.0, std::min({requested_speed_steps_s_,
                              acceleration_limited, braking_limited}));
  const std::uint64_t interval =
      static_cast<std::uint64_t>(std::llround(1000000.0 /
                                               current_speed_steps_s_));
  return std::max(interval, kStepPulseWidthUs + kMinimumStepLowUs);
}

void AxisController::stop_pulse_generation() {
  if (pulse_scheduler_ != nullptr) {
    pulse_scheduler_->stop();
  }
  if (step_high_) {
    driver_.set_step(false);
  }
  step_high_ = false;
  state_.moving = false;
  state_.target_step_position = state_.internal_step_position;
  motion_purpose_ = MotionPurpose::none;
}

void AxisController::finish_motion() {
  stop_pulse_generation();
  state_.commanded_position_deg =
      steps_to_degrees(state_.internal_step_position);
  state_.last_completed_command = active_command_id_;
}

void AxisController::service_step_generator(const std::uint64_t now_us) {
  if (pulse_scheduler_ != nullptr) {
    if (pulse_scheduler_->consume_scheduler_fault()) {
      fail(FaultCode::driver_communication, TrustLossReason::driver_fault,
           true);
      return;
    }
    const std::uint32_t completed = pulse_scheduler_->consume_completed_pulses();
    for (std::uint32_t index = 0; index < completed && state_.moving; ++index) {
      state_.internal_step_position += step_direction_;
      state_.commanded_position_deg =
          steps_to_degrees(state_.internal_step_position);
      const std::int64_t remaining =
          std::llabs(state_.target_step_position - state_.internal_step_position);
      if (remaining == 0) {
        finish_motion();
        break;
      }
      const std::uint64_t interval = step_interval_us(remaining);
      const std::uint64_t low_time_us =
          std::max(kMinimumStepLowUs, interval - kStepPulseWidthUs);
      if (!pulse_scheduler_->schedule_pulse(
              static_cast<std::uint32_t>(low_time_us))) {
        fail(FaultCode::driver_communication, TrustLossReason::driver_fault,
             true);
        break;
      }
    }
    return;
  }
  if (!state_.moving || now_us < next_edge_us_) {
    return;
  }
  if (!step_high_) {
    driver_.set_step(true);
    step_high_ = true;
    next_edge_us_ = now_us + kStepPulseWidthUs;
    return;
  }

  driver_.set_step(false);
  step_high_ = false;
  state_.internal_step_position += step_direction_;
  state_.commanded_position_deg =
      steps_to_degrees(state_.internal_step_position);
  const std::int64_t remaining =
      std::llabs(state_.target_step_position -
                 state_.internal_step_position);
  if (remaining == 0) {
    finish_motion();
    return;
  }
  const std::uint64_t interval = step_interval_us(remaining);
  next_edge_us_ =
      now_us + std::max(kMinimumStepLowUs, interval - kStepPulseWidthUs);
}

void AxisController::service_homing(const std::uint64_t now_us) {
  if (state_.homing_phase == HomingPhase::idle ||
      state_.homing_phase == HomingPhase::complete ||
      state_.homing_phase == HomingPhase::failed) {
    return;
  }
  const std::uint64_t homing_timeout_us =
      static_cast<std::uint64_t>(config_.motion.homing.timeout_ms) * 1000ULL;
  if (elapsed(now_us, homing_started_us_, homing_timeout_us)) {
    const FaultCode fault =
        state_.homing_phase == HomingPhase::fast_approach
            ? FaultCode::homing_switch_never_triggered
            : FaultCode::homing_timeout;
    fail(fault, TrustLossReason::homing_failed, true);
    return;
  }

  if (state_.homing_phase == HomingPhase::fast_approach &&
      state_.home_switch_active) {
    stop_pulse_generation();
    state_.homing_phase = HomingPhase::backoff;
    std::int64_t backoff_steps = 0;
    degrees_to_steps(config_.motion.homing.backoff_deg, backoff_steps);
    const std::int64_t target =
        state_.internal_step_position +
        (config_.motion.homing.direction_negative ? backoff_steps
                                                  : -backoff_steps);
    start_step_move(target, config_.motion.homing.speed_deg_per_s,
                    MotionPurpose::homing_backoff, active_command_id_, false);
    return;
  }

  if (state_.homing_phase == HomingPhase::backoff && !state_.moving) {
    state_.homing_phase = HomingPhase::confirm_release;
    if (state_.home_switch_active) {
      fail(FaultCode::homing_switch_failed_release,
           TrustLossReason::homing_failed, true);
      return;
    }
    state_.homing_phase = HomingPhase::slow_approach;
    std::int64_t backoff_steps = 0;
    degrees_to_steps(config_.motion.homing.backoff_deg, backoff_steps);
    const std::int64_t target =
        state_.internal_step_position +
        (config_.motion.homing.direction_negative
             ? -std::max<std::int64_t>(backoff_steps * 2, 1)
             : std::max<std::int64_t>(backoff_steps * 2, 1));
    start_step_move(target, config_.motion.homing.slow_approach_deg_per_s,
                    MotionPurpose::homing_slow, active_command_id_, false);
    return;
  }

  if (state_.homing_phase == HomingPhase::slow_approach &&
      state_.home_switch_active) {
    stop_pulse_generation();
    state_.homing_phase = HomingPhase::apply_offset;
    std::int64_t offset_steps = 0;
    if (!degrees_to_steps(config_.motion.home_offset_deg, offset_steps)) {
      fail(FaultCode::invalid_configuration,
           TrustLossReason::homing_failed, true);
      return;
    }
    state_.internal_step_position = offset_steps;
    state_.target_step_position = offset_steps;
    state_.commanded_position_deg = config_.motion.home_offset_deg;
    state_.position_trusted = true;
    state_.homed = true;
    state_.trust_loss_reason = TrustLossReason::none;
    state_.fault = FaultCode::none;
    state_.homing_phase = HomingPhase::complete;
    state_.last_completed_command = active_command_id_;
  }
}

void AxisController::service_driver_status(const std::uint64_t now_us) {
  if (!elapsed(now_us, last_status_check_us_, kDriverStatusIntervalUs)) {
    return;
  }
  last_status_check_us_ = now_us;
  state_.last_driver_status = driver_.read_status();
  if (state_.last_driver_status.critical_fault()) {
    fail(state_.last_driver_status.connected
             ? FaultCode::driver_critical
             : FaultCode::driver_communication,
         TrustLossReason::driver_fault, true);
  }
}

void AxisController::service_diagnostics() {
  // TMC UART receives may wait for their bounded timeout.  A scheduler-backed
  // move relies on this task to arm the next one-shot GPTimer pulse, so never
  // perform that I/O while an axis is moving.
  if (state_.moving) {
    return;
  }
  service_driver_status(platform_.monotonic_micros());
}

void AxisController::service() {
  const std::uint64_t now_us = platform_.monotonic_micros();
  update_home_switch(now_us);
  if (state_.moving &&
      elapsed(now_us, motion_started_us_,
              static_cast<std::uint64_t>(
                  config_.motion.motion_timeout_ms) *
                  1000ULL)) {
    fail(FaultCode::motion_timeout, TrustLossReason::motion_timeout, true);
    return;
  }
  if (state_.moving && motion_purpose_ == MotionPurpose::normal &&
      state_.home_switch_active) {
    fail(FaultCode::unexpected_home_switch,
         TrustLossReason::suspected_missed_step, true);
    return;
  }
  service_step_generator(now_us);
  service_homing(now_us);
}

MotionResult AxisController::stop(const bool invalidate_position) {
  const bool was_moving = state_.moving;
  stop_pulse_generation();
  if (invalidate_position && was_moving) {
    mark_position_untrusted(TrustLossReason::stopped);
  }
  state_.fault = FaultCode::stopped;
  return MotionResult{true, FaultCode::stopped};
}

MotionResult AxisController::emergency_stop() {
  stop_pulse_generation();
  driver_.disable();
  state_.enabled = false;
  return fail(FaultCode::emergency_stop,
              TrustLossReason::emergency_stop, true);
}

MotionResult AxisController::set_enabled(const bool enabled) {
  if (!enabled) {
    stop_pulse_generation();
    driver_.disable();
    state_.enabled = false;
    mark_position_untrusted(
        TrustLossReason::driver_disabled_during_motion);
    state_.fault = FaultCode::driver_disabled;
    return MotionResult{true, FaultCode::driver_disabled};
  }
  if (state_.last_driver_status.critical_fault() || !driver_.enable()) {
    return fail(FaultCode::driver_critical,
                TrustLossReason::driver_fault, true);
  }
  state_.enabled = true;
  return succeed();
}

MotionResult AxisController::clear_fault() {
  if (state_.last_driver_status.critical_fault()) {
    return MotionResult{false, FaultCode::driver_critical};
  }
  state_.fault = FaultCode::none;
  if (state_.homing_phase == HomingPhase::failed) {
    state_.homing_phase = HomingPhase::idle;
  }
  return succeed();
}

MotionResult AxisController::set_current(
    const std::uint16_t rms_current_ma) {
  if (state_.moving ||
      !driver_.set_current_milliamps(
          rms_current_ma, config_.motion.hold_current_percent)) {
    return MotionResult{false, FaultCode::invalid_argument};
  }
  config_.motion.motor_rms_current_ma = rms_current_ma;
  return succeed();
}

MotionResult AxisController::set_microsteps(
    const std::uint16_t microsteps) {
  if (state_.moving || !driver_.set_microsteps(microsteps)) {
    return MotionResult{false, FaultCode::invalid_argument};
  }
  if (microsteps != config_.motion.microsteps) {
    config_.motion.microsteps = microsteps;
    mark_position_untrusted(TrustLossReason::configuration_changed);
  }
  return succeed();
}

}  // namespace radiance3d
