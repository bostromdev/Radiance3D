#include "tmc2209_driver.hpp"

#include <algorithm>
#include <cmath>

namespace radiance3d {
namespace {

constexpr std::uint8_t kSync = 0x05;
constexpr std::uint8_t kMasterAddress = 0xFF;
constexpr std::uint8_t kWriteBit = 0x80;
constexpr std::uint8_t kRegisterGconf = 0x00;
constexpr std::uint8_t kRegisterGstat = 0x01;
constexpr std::uint8_t kRegisterIfcnt = 0x02;
constexpr std::uint8_t kRegisterSlaveconf = 0x03;
constexpr std::uint8_t kRegisterIholdIrun = 0x10;
constexpr std::uint8_t kRegisterTpowerdown = 0x11;
constexpr std::uint8_t kRegisterChopconf = 0x6C;
constexpr std::uint8_t kRegisterDrvStatus = 0x6F;
constexpr std::uint8_t kRegisterPwmconf = 0x70;

constexpr std::uint32_t kGconfSpreadcycle = 1UL << 2;
constexpr std::uint32_t kGconfPdnDisable = 1UL << 6;
constexpr std::uint32_t kGconfMstepRegisterSelect = 1UL << 7;
constexpr std::uint32_t kGconfMultistepFilter = 1UL << 8;
constexpr std::uint32_t kChopconfVsense = 1UL << 17;
constexpr std::uint32_t kChopconfMresMask = 0xFUL << 24;
constexpr std::uint32_t kChopconfInterpolation = 1UL << 28;
constexpr std::uint32_t kPwmAutoscale = 1UL << 18;
constexpr std::uint32_t kPwmAutograd = 1UL << 19;

std::uint8_t clamp_scale(const long value) {
  return static_cast<std::uint8_t>(std::max(0L, std::min(31L, value)));
}

}  // namespace

Tmc2209Driver::Tmc2209Driver(HardwarePlatform& platform, Tmc2209Config config)
    : platform_(platform), config_(config) {}

const Tmc2209Config& Tmc2209Driver::config() const { return config_; }

std::uint16_t Tmc2209Driver::configured_current_ma() const {
  return configured_current_ma_;
}

std::uint16_t Tmc2209Driver::configured_microsteps() const {
  return configured_microsteps_;
}

bool Tmc2209Driver::valid_config() const {
  return config_.address <= 3 && config_.uart_channel > 0 &&
         config_.uart_tx_pin >= 0 && config_.uart_rx_pin >= 0 &&
         config_.step_pin >= 0 && config_.direction_pin >= 0 &&
         config_.enable_pin >= 0 && config_.sense_resistor_milliohms > 0 &&
         config_.maximum_rms_current_ma > 0 && config_.uart_baud > 0 &&
         config_.uart_timeout_ms > 0;
}

DriverCapabilities Tmc2209Driver::capabilities() const {
  return DriverCapabilities{true, true, true, true, true, true};
}

std::uint8_t Tmc2209Driver::calculate_crc(const std::uint8_t* bytes,
                                          const std::size_t length_without_crc) {
  std::uint8_t crc = 0;
  for (std::size_t index = 0; index < length_without_crc; ++index) {
    std::uint8_t current = bytes[index];
    for (std::uint8_t bit = 0; bit < 8; ++bit) {
      if (((crc >> 7) ^ (current & 0x01U)) != 0U) {
        crc = static_cast<std::uint8_t>((crc << 1) ^ 0x07U);
      } else {
        crc = static_cast<std::uint8_t>(crc << 1);
      }
      current = static_cast<std::uint8_t>(current >> 1);
    }
  }
  return crc;
}

bool Tmc2209Driver::write_register(const std::uint8_t address,
                                   const std::uint32_t value) {
  std::uint8_t datagram[8] = {
      kSync,
      config_.address,
      static_cast<std::uint8_t>(address | kWriteBit),
      static_cast<std::uint8_t>((value >> 24) & 0xFFU),
      static_cast<std::uint8_t>((value >> 16) & 0xFFU),
      static_cast<std::uint8_t>((value >> 8) & 0xFFU),
      static_cast<std::uint8_t>(value & 0xFFU),
      0,
  };
  datagram[7] = calculate_crc(datagram, 7);
  return platform_.write_uart(config_.uart_channel, datagram, sizeof(datagram));
}

bool Tmc2209Driver::read_register(const std::uint8_t address,
                                  std::uint32_t& value) {
  std::uint8_t request[4] = {kSync, config_.address, address, 0};
  request[3] = calculate_crc(request, 3);
  platform_.flush_uart_input(config_.uart_channel);
  if (!platform_.write_uart(config_.uart_channel, request, sizeof(request))) {
    return false;
  }

  std::uint8_t response[16] = {};
  const std::size_t received =
      platform_.read_uart(config_.uart_channel, response, sizeof(response),
                          config_.uart_timeout_ms);
  if (received < 8) {
    return false;
  }
  for (std::size_t offset = 0; offset + 8 <= received; ++offset) {
    const std::uint8_t* frame = response + offset;
    if (frame[0] != kSync || frame[1] != kMasterAddress ||
        frame[2] != address || calculate_crc(frame, 7) != frame[7]) {
      continue;
    }
    value = (static_cast<std::uint32_t>(frame[3]) << 24) |
            (static_cast<std::uint32_t>(frame[4]) << 16) |
            (static_cast<std::uint32_t>(frame[5]) << 8) |
            static_cast<std::uint32_t>(frame[6]);
    return true;
  }
  return false;
}

bool Tmc2209Driver::verify_write_counter(const std::uint8_t before) {
  std::uint32_t after = 0;
  return read_register(kRegisterIfcnt, after) &&
         static_cast<std::uint8_t>(after) ==
             static_cast<std::uint8_t>(before + 1U);
}

bool Tmc2209Driver::initialize() {
  connected_ = false;
  enabled_ = false;
  if (!valid_config() ||
      !platform_.configure_pin(config_.step_pin, PinMode::output) ||
      !platform_.configure_pin(config_.direction_pin, PinMode::output) ||
      !platform_.configure_pin(config_.enable_pin, PinMode::output)) {
    return false;
  }
  platform_.write_pin(config_.step_pin, false);
  platform_.write_pin(config_.direction_pin, config_.direction_inverted);
  disable();
  if (!platform_.begin_uart(config_.uart_channel, config_.uart_tx_pin,
                            config_.uart_rx_pin, config_.uart_baud)) {
    return false;
  }

  std::uint32_t ifcnt = 0;
  if (!read_register(kRegisterIfcnt, ifcnt)) {
    return false;
  }
  gconf_ = kGconfPdnDisable | kGconfMstepRegisterSelect |
           kGconfMultistepFilter;
  if (!write_register(kRegisterGconf, gconf_) ||
      !verify_write_counter(static_cast<std::uint8_t>(ifcnt))) {
    return false;
  }
  if (!write_register(kRegisterSlaveconf, 2UL << 8) ||
      !write_register(kRegisterTpowerdown, 10) ||
      !write_register(kRegisterPwmconf,
                      0xC10D0024UL | kPwmAutoscale | kPwmAutograd)) {
    return false;
  }
  connected_ = true;
  return set_microsteps(configured_microsteps_) &&
         set_interpolation(interpolation_) &&
         set_chopper_mode(chopper_mode_);
}

bool Tmc2209Driver::enable() {
  if (!connected_) {
    return false;
  }
  platform_.write_pin(config_.enable_pin, !config_.enable_active_low);
  enabled_ = true;
  return true;
}

void Tmc2209Driver::disable() {
  platform_.write_pin(config_.enable_pin, config_.enable_active_low);
  platform_.write_pin(config_.step_pin, false);
  enabled_ = false;
}

bool Tmc2209Driver::set_direction(const bool positive) {
  if (!connected_) {
    return false;
  }
  platform_.write_pin(config_.direction_pin,
                      positive != config_.direction_inverted);
  return true;
}

void Tmc2209Driver::set_step(const bool high) {
  platform_.write_pin(config_.step_pin, high);
}

bool Tmc2209Driver::set_current_milliamps(
    const std::uint16_t rms_current_ma, const std::uint8_t hold_percent) {
  if (!connected_ || rms_current_ma == 0 ||
      rms_current_ma > config_.maximum_rms_current_ma ||
      hold_percent > 100) {
    return false;
  }

  const double resistance_ohms =
      static_cast<double>(config_.sense_resistor_milliohms) / 1000.0 + 0.02;
  const double current_amps = static_cast<double>(rms_current_ma) / 1000.0;
  double voltage = 0.325;
  long run_scale = std::lround(32.0 * 1.41421356237 * current_amps *
                                  resistance_ohms / voltage -
                              1.0);
  if (run_scale < 16) {
    voltage = 0.180;
    run_scale = std::lround(32.0 * 1.41421356237 * current_amps *
                                resistance_ohms / voltage -
                            1.0);
    chopconf_ |= kChopconfVsense;
  } else {
    chopconf_ &= ~kChopconfVsense;
  }
  if (run_scale < 0 || run_scale > 31) {
    return false;
  }
  const std::uint8_t run = clamp_scale(run_scale);
  const long hold_scale =
      std::lround((static_cast<double>(run + 1U) * hold_percent / 100.0) - 1.0);
  const std::uint8_t hold = clamp_scale(hold_scale);
  const std::uint32_t ihold_irun =
      static_cast<std::uint32_t>(hold) |
      (static_cast<std::uint32_t>(run) << 8) | (6UL << 16);
  if (!write_register(kRegisterChopconf, chopconf_) ||
      !write_register(kRegisterIholdIrun, ihold_irun)) {
    connected_ = false;
    disable();
    return false;
  }
  configured_current_ma_ = rms_current_ma;
  return true;
}

bool Tmc2209Driver::microstep_code(const std::uint16_t microsteps,
                                   std::uint8_t& code) {
  switch (microsteps) {
    case 256:
      code = 0;
      return true;
    case 128:
      code = 1;
      return true;
    case 64:
      code = 2;
      return true;
    case 32:
      code = 3;
      return true;
    case 16:
      code = 4;
      return true;
    case 8:
      code = 5;
      return true;
    case 4:
      code = 6;
      return true;
    case 2:
      code = 7;
      return true;
    case 1:
      code = 8;
      return true;
    default:
      return false;
  }
}

bool Tmc2209Driver::set_microsteps(const std::uint16_t microsteps) {
  std::uint8_t code = 0;
  if (!connected_ || !microstep_code(microsteps, code)) {
    return false;
  }
  chopconf_ = (chopconf_ & ~kChopconfMresMask) |
              (static_cast<std::uint32_t>(code) << 24);
  if (!write_register(kRegisterChopconf, chopconf_)) {
    connected_ = false;
    disable();
    return false;
  }
  configured_microsteps_ = microsteps;
  return true;
}

bool Tmc2209Driver::set_interpolation(const bool enabled) {
  if (!connected_) {
    return false;
  }
  interpolation_ = enabled;
  if (enabled) {
    chopconf_ |= kChopconfInterpolation;
  } else {
    chopconf_ &= ~kChopconfInterpolation;
  }
  return write_register(kRegisterChopconf, chopconf_);
}

bool Tmc2209Driver::set_chopper_mode(const ChopperMode mode) {
  if (!connected_) {
    return false;
  }
  chopper_mode_ = mode;
  if (mode == ChopperMode::spreadcycle) {
    gconf_ |= kGconfSpreadcycle;
  } else {
    gconf_ &= ~kGconfSpreadcycle;
  }
  return write_register(kRegisterGconf, gconf_);
}

DriverFault Tmc2209Driver::primary_fault(const DriverStatus& status) {
  if (!status.connected) {
    return DriverFault::communication_failure;
  }
  if (status.overtemperature_shutdown) {
    return DriverFault::overtemperature_shutdown;
  }
  if (status.short_to_ground_a || status.short_to_ground_b) {
    return DriverFault::short_to_ground;
  }
  if (status.short_to_supply_a || status.short_to_supply_b) {
    return DriverFault::short_to_supply;
  }
  if (status.undervoltage) {
    return DriverFault::undervoltage;
  }
  if (status.overtemperature_warning) {
    return DriverFault::overtemperature_warning;
  }
  if (status.reset_detected) {
    return DriverFault::reset_detected;
  }
  if (status.open_load_a || status.open_load_b) {
    return DriverFault::open_load;
  }
  return DriverFault::none;
}

DriverStatus Tmc2209Driver::read_status() {
  DriverStatus status;
  status.connected = connected_;
  status.enabled = enabled_;
  std::uint32_t gstat = 0;
  std::uint32_t driver = 0;
  if (!connected_ || !read_register(kRegisterGstat, gstat) ||
      !read_register(kRegisterDrvStatus, driver)) {
    connected_ = false;
    disable();
    status.connected = false;
    status.enabled = false;
    status.fault = DriverFault::communication_failure;
    return status;
  }
  status.reset_detected = (gstat & (1UL << 0)) != 0;
  status.undervoltage = (gstat & (1UL << 2)) != 0;
  status.standstill = (driver & (1UL << 31)) != 0;
  status.stealthchop_active = (driver & (1UL << 30)) != 0;
  status.current_scale = static_cast<std::uint8_t>((driver >> 16) & 0x1FU);
  status.overtemperature_warning = (driver & (1UL << 0)) != 0;
  status.overtemperature_shutdown = (driver & (1UL << 1)) != 0;
  status.short_to_ground_a = (driver & (1UL << 2)) != 0;
  status.short_to_ground_b = (driver & (1UL << 3)) != 0;
  status.short_to_supply_a = (driver & (1UL << 4)) != 0;
  status.short_to_supply_b = (driver & (1UL << 5)) != 0;
  status.open_load_a = (driver & (1UL << 6)) != 0;
  status.open_load_b = (driver & (1UL << 7)) != 0;
  status.fault = primary_fault(status);
  if (status.critical_fault()) {
    disable();
    status.enabled = false;
  }
  return status;
}

bool Tmc2209Driver::is_connected() const { return connected_; }

}  // namespace radiance3d
