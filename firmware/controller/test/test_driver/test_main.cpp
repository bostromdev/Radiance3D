#ifdef ESP_PLATFORM
#include <unity.h>
#else
#include "host_test.hpp"
#endif

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

#include "hardware_platform.hpp"
#include "tmc2209_driver.hpp"

namespace {

class FakePlatform final : public radiance3d::HardwarePlatform {
 public:
  bool uart_present{true};
  bool uart_started{false};
  bool half_duplex_requested{false};
  bool echo_read_requests{false};
  bool pin_values[64]{};
  std::array<std::uint32_t, 128> registers{};

  bool configure_pin(int pin, radiance3d::PinMode) override {
    return pin >= 0 && pin < 64;
  }

  void write_pin(int pin, bool high) override {
    if (pin >= 0 && pin < 64) {
      pin_values[pin] = high;
    }
  }

  bool read_pin(int pin) const override {
    return pin >= 0 && pin < 64 && pin_values[pin];
  }

  std::uint64_t monotonic_micros() const override { return 0; }

  bool begin_uart(std::uint8_t, int, int, std::uint32_t) override {
    uart_started = true;
    return true;
  }

  bool configure_uart_half_duplex(std::uint8_t, bool enabled) override {
    half_duplex_requested = enabled;
    return true;
  }

  void flush_uart_input(std::uint8_t) override {}

  bool write_uart(std::uint8_t, const std::uint8_t* data,
                  std::size_t length) override {
    if (!uart_present) {
      return false;
    }
    if (length == 8 && data[0] == 0x05 && data[1] <= 3 &&
        radiance3d::Tmc2209Driver::calculate_crc(data, 7) == data[7]) {
      const std::uint8_t address = static_cast<std::uint8_t>(data[2] & 0x7FU);
      const std::uint32_t value =
          (static_cast<std::uint32_t>(data[3]) << 24) |
          (static_cast<std::uint32_t>(data[4]) << 16) |
          (static_cast<std::uint32_t>(data[5]) << 8) |
          static_cast<std::uint32_t>(data[6]);
      if (address == 0x01) {
        registers[address] &= ~value;
      } else {
        registers[address] = value;
      }
      registers[0x02] = static_cast<std::uint8_t>(registers[0x02] + 1U);
      pending_register_ = address;
      return true;
    }
    if (length == 4 && data[0] == 0x05 && data[1] <= 3 &&
        radiance3d::Tmc2209Driver::calculate_crc(data, 3) == data[3]) {
      pending_register_ = data[2];
      if (echo_read_requests) {
        std::copy(data, data + length, echoed_request_.begin());
        echo_pending_ = true;
      }
      return true;
    }
    return false;
  }

  std::size_t read_uart(std::uint8_t, std::uint8_t* data,
                        std::size_t maximum_length, std::uint32_t) override {
    if (!uart_present) {
      return 0;
    }
    if (echo_pending_) {
      if (maximum_length < echoed_request_.size()) {
        return 0;
      }
      std::copy(echoed_request_.begin(), echoed_request_.end(), data);
      echo_pending_ = false;
      return echoed_request_.size();
    }
    if (maximum_length < 8) {
      return 0;
    }
    const std::uint32_t value = registers[pending_register_];
    data[0] = 0x05;
    data[1] = 0xFF;
    data[2] = pending_register_;
    data[3] = static_cast<std::uint8_t>((value >> 24) & 0xFFU);
    data[4] = static_cast<std::uint8_t>((value >> 16) & 0xFFU);
    data[5] = static_cast<std::uint8_t>((value >> 8) & 0xFFU);
    data[6] = static_cast<std::uint8_t>(value & 0xFFU);
    data[7] = radiance3d::Tmc2209Driver::calculate_crc(data, 7);
    return 8;
  }

 private:
  std::uint8_t pending_register_{0};
  std::array<std::uint8_t, 4> echoed_request_{};
  bool echo_pending_{false};
};

radiance3d::Tmc2209Config config() {
  radiance3d::Tmc2209Config value;
  value.uart_channel = 1;
  value.address = 0;
  value.uart_tx_pin = 17;
  value.uart_rx_pin = 16;
  value.step_pin = 25;
  value.direction_pin = 26;
  value.enable_pin = 27;
  value.sense_resistor_milliohms = 110;
  value.maximum_rms_current_ma = 800;
  return value;
}

}  // namespace

void setUp() {}
void tearDown() {}

void test_successful_initialization_starts_disabled_and_probes_uart() {
  FakePlatform platform;
  platform.registers[0x01] = 1U;
  radiance3d::Tmc2209Driver driver(platform, config());

  TEST_ASSERT_TRUE(driver.initialize());
  TEST_ASSERT_TRUE(driver.is_connected());
  TEST_ASSERT_TRUE(platform.uart_started);
  TEST_ASSERT_TRUE(platform.half_duplex_requested);
  TEST_ASSERT_TRUE(platform.pin_values[27]);
  TEST_ASSERT_EQUAL_UINT32(0, platform.registers[0x01]);
}

void test_single_wire_read_echo_is_ignored_before_crc_valid_reply() {
  FakePlatform platform;
  platform.echo_read_requests = true;
  radiance3d::Tmc2209Driver driver(platform, config());

  TEST_ASSERT_TRUE(driver.initialize());
  TEST_ASSERT_TRUE(driver.set_microsteps(32));
}

void test_failed_uart_probe_keeps_driver_disabled() {
  FakePlatform platform;
  platform.uart_present = false;
  radiance3d::Tmc2209Driver driver(platform, config());

  TEST_ASSERT_FALSE(driver.initialize());
  TEST_ASSERT_FALSE(driver.is_connected());
  TEST_ASSERT_TRUE(platform.pin_values[27]);
}

void test_invalid_driver_address_is_rejected() {
  FakePlatform platform;
  auto invalid = config();
  invalid.address = 4;
  radiance3d::Tmc2209Driver driver(platform, invalid);

  TEST_ASSERT_FALSE(driver.initialize());
  TEST_ASSERT_FALSE(driver.enable());
}

void test_current_is_configurable_and_safe_ceiling_is_enforced() {
  FakePlatform platform;
  radiance3d::Tmc2209Driver driver(platform, config());
  TEST_ASSERT_TRUE(driver.initialize());

  TEST_ASSERT_TRUE(driver.set_current_milliamps(400, 30));
  TEST_ASSERT_EQUAL_UINT16(400, driver.configured_current_ma());
  TEST_ASSERT_FALSE(driver.set_current_milliamps(801, 30));
  TEST_ASSERT_FALSE(driver.set_current_milliamps(400, 101));
}

void test_supported_microsteps_are_written_and_unsupported_values_rejected() {
  FakePlatform platform;
  radiance3d::Tmc2209Driver driver(platform, config());
  TEST_ASSERT_TRUE(driver.initialize());

  TEST_ASSERT_TRUE(driver.set_microsteps(32));
  TEST_ASSERT_EQUAL_UINT16(32, driver.configured_microsteps());
  TEST_ASSERT_FALSE(driver.set_microsteps(3));
}

void test_diagnostics_map_faults_and_critical_fault_disables_output() {
  FakePlatform platform;
  radiance3d::Tmc2209Driver driver(platform, config());
  TEST_ASSERT_TRUE(driver.initialize());
  TEST_ASSERT_TRUE(driver.enable());
  platform.registers[0x01] = 1U << 1;
  platform.registers[0x6F] = (1UL << 1) | (1UL << 2);

  const radiance3d::DriverStatus status = driver.read_status();

  TEST_ASSERT_TRUE(status.overtemperature_shutdown);
  TEST_ASSERT_TRUE(status.short_to_ground_a);
  TEST_ASSERT_TRUE(status.critical_fault());
  TEST_ASSERT_EQUAL(radiance3d::DriverFault::overtemperature_shutdown,
                    status.fault);
  TEST_ASSERT_TRUE(platform.pin_values[27]);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_successful_initialization_starts_disabled_and_probes_uart);
  RUN_TEST(test_single_wire_read_echo_is_ignored_before_crc_valid_reply);
  RUN_TEST(test_failed_uart_probe_keeps_driver_disabled);
  RUN_TEST(test_invalid_driver_address_is_rejected);
  RUN_TEST(test_current_is_configurable_and_safe_ceiling_is_enforced);
  RUN_TEST(test_supported_microsteps_are_written_and_unsupported_values_rejected);
  RUN_TEST(test_diagnostics_map_faults_and_critical_fault_disables_output);
  return UNITY_END();
}
