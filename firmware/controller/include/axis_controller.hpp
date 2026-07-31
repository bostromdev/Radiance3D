#pragma once

#include "hardware_platform.hpp"
#include "motion_controller.hpp"
#include "step_pulse_scheduler.hpp"
#include "stepper_driver.hpp"

#include <cstdint>

namespace radiance3d {

struct PhysicalAxisConfig {
  const char* name{"axis"};
  AxisConfig motion{};
  int home_switch_pin{-1};
  PinMode home_switch_input_mode{PinMode::input_pullup};
};

class AxisController {
 public:
  AxisController(HardwarePlatform& platform, StepperDriver& driver,
                 PhysicalAxisConfig config,
                 StepPulseScheduler* pulse_scheduler = nullptr);

  bool initialize();
  void service();
  // Runs bounded, potentially blocking driver diagnostics only while this
  // axis is idle.  The physical runtime invokes it from its diagnostics tick
  // so UART timeouts can never delay scheduling the next STEP pulse.
  void service_diagnostics();
  MotionResult start_homing(std::uint32_t command_id = 0);
  MotionResult move_absolute_degrees(double target_deg, double speed_deg_per_s,
                                     std::uint32_t command_id = 0);
  MotionResult move_relative_degrees(double delta_deg, double speed_deg_per_s,
                                     std::uint32_t command_id = 0);
  MotionResult bench_move_steps(std::int64_t signed_steps,
                                std::uint32_t command_id = 0);
  MotionResult stop(bool invalidate_position = true);
  MotionResult emergency_stop();
  MotionResult set_enabled(bool enabled);
  MotionResult clear_fault();
  MotionResult set_current(std::uint16_t rms_current_ma);
  MotionResult set_microsteps(std::uint16_t microsteps);
  void mark_position_untrusted(TrustLossReason reason);

  const PhysicalAxisConfig& config() const;
  const AxisState& state() const;
  AxisState& mutable_state();
  DriverCapabilities driver_capabilities() const;
  DriverStatus driver_status() const;

  bool degrees_to_steps(double degrees, std::int64_t& steps) const;
  double steps_to_degrees(std::int64_t steps) const;
  bool motor_full_steps_to_output_steps(std::int64_t motor_full_steps,
                                        std::int64_t& output_steps) const;

 private:
  enum class MotionPurpose { none, normal, bench, homing_fast, homing_backoff, homing_slow };

  HardwarePlatform& platform_;
  StepperDriver& driver_;
  PhysicalAxisConfig config_;
  StepPulseScheduler* pulse_scheduler_{nullptr};
  AxisState state_{};
  MotionPurpose motion_purpose_{MotionPurpose::none};
  std::uint64_t motion_started_us_{0};
  std::uint64_t homing_started_us_{0};
  std::uint64_t next_edge_us_{0};
  std::uint64_t last_status_check_us_{0};
  bool step_high_{false};
  int step_direction_{0};
  double current_speed_steps_s_{0.0};
  double requested_speed_steps_s_{0.0};
  bool raw_home_candidate_{false};
  bool debounced_home_active_{false};
  bool home_input_initialized_{false};
  std::uint64_t home_candidate_changed_us_{0};
  std::uint32_t active_command_id_{0};

  MotionResult fail(FaultCode fault, TrustLossReason reason,
                    bool disable_driver);
  MotionResult succeed();
  MotionResult start_step_move(std::int64_t target_steps,
                               double speed_deg_per_s,
                               MotionPurpose purpose,
                               std::uint32_t command_id,
                               bool require_trust);
  void service_step_generator(std::uint64_t now_us);
  void service_homing(std::uint64_t now_us);
  void service_driver_status(std::uint64_t now_us);
  void finish_motion();
  void stop_pulse_generation();
  void update_home_switch(std::uint64_t now_us);
  bool home_switch_active_raw() const;
  std::uint64_t step_interval_us(std::int64_t remaining_steps);
  std::int64_t minimum_steps() const;
  std::int64_t maximum_steps() const;
};

}  // namespace radiance3d
