#pragma once

#include "axis_controller.hpp"
#include "motion_controller.hpp"
#include "tmc2209_driver.hpp"

#include <cstdint>

namespace radiance3d {

struct PhysicalAxisDefinition {
  PhysicalAxisConfig axis{};
  Tmc2209Config driver{};
};

struct PhysicalControllerConfig {
  const char* board_name{"unassigned"};
  std::uint32_t protocol_version{1};
  PhysicalAxisDefinition azimuth{};
  PhysicalAxisDefinition elevation{};
  int emergency_stop_pin{-1};
  bool emergency_stop_active_low{true};
  std::uint32_t emergency_stop_debounce_ms{10};
};

struct GpioValidationResult {
  bool valid{false};
  int duplicate_pin{-1};
  int invalid_output_pin{-1};
  std::uint64_t bootstrapping_pin_mask{0};
};

GpioValidationResult validate_esp32_gpio(
    const PhysicalControllerConfig& config);
PhysicalControllerConfig provisional_esp32_dev_config();

}  // namespace radiance3d
