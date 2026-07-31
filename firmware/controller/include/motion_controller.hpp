#pragma once

#include "stepper_driver.hpp"

#include <cstdint>

namespace radiance3d {

enum class AxisSelection { azimuth, elevation, both };

enum class FaultCode {
  none,
  invalid_command,
  invalid_argument,
  invalid_configuration,
  not_homed,
  position_untrusted,
  limit_reached,
  motion_timeout,
  driver_disabled,
  emergency_stop,
  driver_communication,
  driver_critical,
  homing_stuck_switch,
  homing_switch_never_triggered,
  homing_switch_failed_release,
  homing_timeout,
  unexpected_home_switch,
  stopped,
};

enum class TrustLossReason {
  startup,
  none,
  reset,
  power_loss,
  emergency_stop,
  driver_fault,
  driver_disabled_during_motion,
  motion_timeout,
  suspected_missed_step,
  manual_movement,
  configuration_changed,
  homing_failed,
  watchdog_reset_during_motion,
  stopped,
};

enum class HomingPhase {
  idle,
  validate_switch,
  fast_approach,
  backoff,
  confirm_release,
  slow_approach,
  apply_offset,
  complete,
  failed,
};

struct HomingConfig {
  bool switch_normally_closed{true};
  bool direction_negative{true};
  std::uint32_t debounce_ms{10};
  double speed_deg_per_s{5.0};
  double backoff_deg{2.0};
  double slow_approach_deg_per_s{1.0};
  std::uint32_t timeout_ms{60000};
};

struct AxisConfig {
  std::uint16_t motor_full_steps_per_revolution{200};
  std::uint16_t microsteps{16};
  std::uint16_t motor_rms_current_ma{0};
  std::uint8_t hold_current_percent{30};
  double gear_ratio{1.0};
  bool direction_inverted{false};
  double home_offset_deg{0.0};
  double minimum_angle_deg{0.0};
  double maximum_angle_deg{360.0};
  double maximum_speed_deg_per_s{20.0};
  double acceleration_deg_per_s2{40.0};
  std::uint32_t settling_time_ms{250};
  std::uint32_t motion_timeout_ms{60000};
  std::int64_t maximum_bench_test_steps{3200};
  HomingConfig homing{};

  double steps_per_output_revolution() const;
  double commanded_step_angle_deg() const;
  bool valid() const;
};

struct ControllerConfig {
  AxisConfig azimuth{};
  AxisConfig elevation{};
  std::uint32_t motion_timeout_ms{120000};
  bool emergency_stop_active_low{true};

  bool valid() const;
};

struct AxisState {
  double commanded_position_deg{0.0};
  std::int64_t internal_step_position{0};
  std::int64_t target_step_position{0};
  bool homed{false};
  bool position_trusted{false};
  TrustLossReason trust_loss_reason{TrustLossReason::startup};
  bool moving{false};
  bool enabled{false};
  bool home_switch_active{false};
  FaultCode fault{FaultCode::none};
  HomingPhase homing_phase{HomingPhase::idle};
  DriverStatus last_driver_status{};
  std::uint32_t last_completed_command{0};
};

struct ControllerState {
  AxisState azimuth{};
  AxisState elevation{};
  FaultCode fault{FaultCode::none};
  bool stopped{false};
  bool emergency_stop_active{false};
};

struct MotionResult {
  bool ok;
  FaultCode fault;

  constexpr MotionResult(bool ok_value = false,
                         FaultCode fault_value = FaultCode::none)
      : ok(ok_value), fault(fault_value) {}
};

class MotionController {
 public:
  virtual ~MotionController() = default;

  virtual bool initialize() = 0;
  virtual void service() = 0;
  virtual const ControllerConfig& config() const = 0;
  virtual const ControllerState& state() const = 0;
  virtual MotionResult home(AxisSelection axis,
                            std::uint32_t command_id = 0) = 0;
  virtual MotionResult move_absolute(double azimuth_deg, double elevation_deg,
                                     double speed_deg_per_s,
                                     std::uint32_t command_id = 0) = 0;
  virtual MotionResult stop() = 0;
  virtual MotionResult emergency_stop() = 0;
  virtual MotionResult clear_fault() = 0;
  virtual MotionResult set_enabled(bool enabled) = 0;
  virtual MotionResult move_relative(AxisSelection axis, double delta_deg,
                                     double speed_deg_per_s,
                                     std::uint32_t command_id = 0) = 0;
  virtual MotionResult bench_move_steps(AxisSelection axis,
                                        std::int64_t signed_steps,
                                        std::uint32_t command_id = 0) = 0;
  virtual MotionResult stop_axis(AxisSelection axis) = 0;
  virtual MotionResult set_axis_enabled(AxisSelection axis, bool enabled) = 0;
  virtual MotionResult set_axis_current(AxisSelection axis,
                                        std::uint16_t rms_current_ma) = 0;
  virtual MotionResult set_axis_microsteps(AxisSelection axis,
                                           std::uint16_t microsteps) = 0;
  virtual DriverCapabilities driver_capabilities(AxisSelection axis) const = 0;
  virtual DriverStatus driver_status(AxisSelection axis) const = 0;
  virtual void report_fault(FaultCode code) = 0;
};

class SimulatedMotionController final : public MotionController {
 public:
  explicit SimulatedMotionController(ControllerConfig config = {});

  bool initialize() override;
  void service() override;
  const ControllerConfig& config() const override;
  const ControllerState& state() const override;
  MotionResult home(AxisSelection axis,
                    std::uint32_t command_id = 0) override;
  MotionResult move_absolute(double azimuth_deg, double elevation_deg,
                             double speed_deg_per_s,
                             std::uint32_t command_id = 0) override;
  MotionResult stop() override;
  MotionResult emergency_stop() override;
  MotionResult clear_fault() override;
  MotionResult set_enabled(bool enabled) override;
  MotionResult move_relative(AxisSelection axis, double delta_deg,
                             double speed_deg_per_s,
                             std::uint32_t command_id = 0) override;
  MotionResult bench_move_steps(AxisSelection axis,
                                std::int64_t signed_steps,
                                std::uint32_t command_id = 0) override;
  MotionResult stop_axis(AxisSelection axis) override;
  MotionResult set_axis_enabled(AxisSelection axis, bool enabled) override;
  MotionResult set_axis_current(AxisSelection axis,
                                std::uint16_t rms_current_ma) override;
  MotionResult set_axis_microsteps(AxisSelection axis,
                                   std::uint16_t microsteps) override;
  DriverCapabilities driver_capabilities(AxisSelection axis) const override;
  DriverStatus driver_status(AxisSelection axis) const override;
  void report_fault(FaultCode code) override;

  void simulate_driver_status(AxisSelection axis, DriverStatus status);
  void simulate_homing_failure(AxisSelection axis, FaultCode fault);
  void simulate_reset(AxisSelection axis);

 private:
  ControllerConfig config_{};
  ControllerState state_{};
  FaultCode azimuth_homing_failure_{FaultCode::none};
  FaultCode elevation_homing_failure_{FaultCode::none};

  MotionResult fail(FaultCode code);
  MotionResult succeed();
  static void invalidate(AxisState& axis, TrustLossReason reason);
};

ControllerConfig provisional_simulator_config();

}  // namespace radiance3d
