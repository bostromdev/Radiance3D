#pragma once

#include "axis_controller.hpp"
#include "hardware_config.hpp"
#include "hardware_platform.hpp"
#include "motion_controller.hpp"

#include <cstdint>

namespace radiance3d {

class PhysicalMotionController final : public MotionController {
 public:
  PhysicalMotionController(HardwarePlatform& platform, AxisController& azimuth,
                           AxisController& elevation,
                           PhysicalControllerConfig physical_config);

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

  const PhysicalControllerConfig& physical_config() const;

 private:
  HardwarePlatform& platform_;
  AxisController& azimuth_;
  AxisController& elevation_;
  PhysicalControllerConfig physical_config_;
  ControllerConfig config_{};
  ControllerState state_{};
  bool initialized_{false};
  bool coordinated_move_active_{false};
  bool emergency_latched_{false};
  bool emergency_candidate_{false};
  bool emergency_stable_{false};
  bool emergency_input_initialized_{false};
  std::uint64_t emergency_changed_us_{0};

  AxisController* selected_axis(AxisSelection axis);
  const AxisController* selected_axis(AxisSelection axis) const;
  void synchronize_state();
  void update_emergency_input(std::uint64_t now_us);
  bool emergency_input_active_raw() const;
  MotionResult reject(FaultCode fault);
};

}  // namespace radiance3d
