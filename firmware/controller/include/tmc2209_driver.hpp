#pragma once

#include "hardware_platform.hpp"
#include "stepper_driver.hpp"

#include <cstddef>
#include <cstdint>

namespace radiance3d {

struct Tmc2209Config {
  std::uint8_t uart_channel{1};
  std::uint8_t address{0};
  int uart_tx_pin{-1};
  int uart_rx_pin{-1};
  int step_pin{-1};
  int direction_pin{-1};
  int enable_pin{-1};
  bool enable_active_low{true};
  bool direction_inverted{false};
  std::uint16_t sense_resistor_milliohms{110};
  std::uint16_t maximum_rms_current_ma{800};
  std::uint32_t uart_baud{115200};
  std::uint32_t uart_timeout_ms{20};
  bool uart_single_wire{true};
  bool write_echo_expected{true};
};

class Tmc2209Driver final : public StepperDriver {
 public:
  Tmc2209Driver(HardwarePlatform& platform, Tmc2209Config config);

  bool initialize() override;
  DriverCapabilities capabilities() const override;
  bool enable() override;
  void disable() override;
  bool set_direction(bool positive) override;
  void set_step(bool high) override;
  bool set_current_milliamps(std::uint16_t rms_current_ma,
                             std::uint8_t hold_percent) override;
  bool set_microsteps(std::uint16_t microsteps) override;
  bool set_interpolation(bool enabled) override;
  bool set_chopper_mode(ChopperMode mode) override;
  DriverStatus read_status() override;
  bool is_connected() const override;

  const Tmc2209Config& config() const;
  std::uint16_t configured_current_ma() const;
  std::uint16_t configured_microsteps() const;
  static std::uint8_t calculate_crc(const std::uint8_t* bytes,
                                    std::size_t length_without_crc);

 private:
  HardwarePlatform& platform_;
  Tmc2209Config config_;
  bool connected_{false};
  bool enabled_{false};
  bool interpolation_{true};
  ChopperMode chopper_mode_{ChopperMode::stealthchop};
  std::uint16_t configured_current_ma_{0};
  std::uint16_t configured_microsteps_{16};
  std::uint32_t gconf_{0};
  std::uint32_t chopconf_{0x10000053UL};

  bool valid_config() const;
  bool write_register(std::uint8_t address, std::uint32_t value);
  bool write_register_verified(std::uint8_t address, std::uint32_t value);
  bool read_register(std::uint8_t address, std::uint32_t& value);
  bool verify_write_counter(std::uint8_t before);
  static bool microstep_code(std::uint16_t microsteps, std::uint8_t& code);
  static DriverFault primary_fault(const DriverStatus& status);
};

}  // namespace radiance3d
