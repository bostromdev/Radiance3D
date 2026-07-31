#pragma once

#include <cstdint>

namespace radiance3d {

enum class ChopperMode { stealthchop, spreadcycle };

enum class DriverFault {
  none,
  invalid_configuration,
  communication_failure,
  reset_detected,
  undervoltage,
  overtemperature_warning,
  overtemperature_shutdown,
  short_to_ground,
  short_to_supply,
  open_load,
};

struct DriverCapabilities {
  bool uart_diagnostics{false};
  bool configurable_current{false};
  bool configurable_microsteps{false};
  bool interpolation{false};
  bool stealthchop{false};
  bool spreadcycle{false};
};

struct DriverStatus {
  bool connected{false};
  bool enabled{false};
  bool standstill{true};
  bool stealthchop_active{false};
  bool reset_detected{false};
  bool undervoltage{false};
  bool overtemperature_warning{false};
  bool overtemperature_shutdown{false};
  bool short_to_ground_a{false};
  bool short_to_ground_b{false};
  bool short_to_supply_a{false};
  bool short_to_supply_b{false};
  bool open_load_a{false};
  bool open_load_b{false};
  std::uint8_t current_scale{0};
  DriverFault fault{DriverFault::none};

  bool critical_fault() const {
    return !connected || undervoltage || overtemperature_shutdown ||
           short_to_ground_a || short_to_ground_b || short_to_supply_a ||
           short_to_supply_b;
  }
};

class StepperDriver {
 public:
  virtual ~StepperDriver() = default;

  virtual bool initialize() = 0;
  virtual DriverCapabilities capabilities() const = 0;
  virtual bool enable() = 0;
  virtual void disable() = 0;
  virtual bool set_direction(bool positive) = 0;
  virtual void set_step(bool high) = 0;
  virtual bool set_current_milliamps(std::uint16_t rms_current_ma,
                                     std::uint8_t hold_percent) = 0;
  virtual bool set_microsteps(std::uint16_t microsteps) = 0;
  virtual bool set_interpolation(bool enabled) = 0;
  virtual bool set_chopper_mode(ChopperMode mode) = 0;
  virtual DriverStatus read_status() = 0;
  virtual bool is_connected() const = 0;
};

}  // namespace radiance3d
