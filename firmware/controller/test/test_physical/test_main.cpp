#include <unity.h>

#include <cstddef>
#include <cstdint>

#include "axis_controller.hpp"
#include "hardware_config.hpp"
#include "physical_motion_controller.hpp"

namespace {

class FakePlatform final : public radiance3d::HardwarePlatform {
 public:
  std::uint64_t now_us{0};
  bool pins[64]{};

  bool configure_pin(int pin, radiance3d::PinMode mode) override {
    if (pin < 0 || pin >= 64) {
      return false;
    }
    if (mode == radiance3d::PinMode::input_pullup) {
      pins[pin] = true;
    }
    return true;
  }
  void write_pin(int pin, bool high) override { pins[pin] = high; }
  bool read_pin(int pin) const override { return pins[pin]; }
  std::uint64_t monotonic_micros() const override { return now_us; }
  bool begin_uart(std::uint8_t, int, int, std::uint32_t) override {
    return true;
  }
  void flush_uart_input(std::uint8_t) override {}
  bool write_uart(std::uint8_t, const std::uint8_t*, std::size_t) override {
    return true;
  }
  std::size_t read_uart(std::uint8_t, std::uint8_t*, std::size_t,
                        std::uint32_t) override {
    return 0;
  }
  void advance(std::uint64_t amount_us) { now_us += amount_us; }
};

class FakeDriver final : public radiance3d::StepperDriver {
 public:
  bool connected{true};
  bool enabled{false};
  radiance3d::DriverStatus status{};

  FakeDriver() { status.connected = true; }
  bool initialize() override { return connected; }
  radiance3d::DriverCapabilities capabilities() const override {
    return {true, true, true, true, true, true};
  }
  bool enable() override {
    enabled = connected;
    return enabled;
  }
  void disable() override { enabled = false; }
  bool set_direction(bool) override { return connected; }
  void set_step(bool) override {}
  bool set_current_milliamps(std::uint16_t value,
                             std::uint8_t hold) override {
    return connected && value > 0 && value <= 800 && hold <= 100;
  }
  bool set_microsteps(std::uint16_t value) override {
    return connected && value > 0;
  }
  bool set_interpolation(bool) override { return connected; }
  bool set_chopper_mode(radiance3d::ChopperMode) override {
    return connected;
  }
  radiance3d::DriverStatus read_status() override {
    status.connected = connected;
    status.enabled = enabled;
    if (!connected) {
      status.fault = radiance3d::DriverFault::communication_failure;
    }
    return status;
  }
  bool is_connected() const override { return connected; }
};

struct Fixture {
  FakePlatform platform;
  FakeDriver azimuth_driver;
  FakeDriver elevation_driver;
  radiance3d::PhysicalControllerConfig config;
  radiance3d::AxisController azimuth;
  radiance3d::AxisController elevation;
  radiance3d::PhysicalMotionController controller;

  Fixture()
      : config(radiance3d::provisional_esp32_dev_config()),
        azimuth(platform, azimuth_driver, configure_axis(config.azimuth.axis)),
        elevation(platform, elevation_driver,
                  configure_axis(config.elevation.axis)),
        controller(platform, azimuth, elevation, configure_controller(config)) {}

  static radiance3d::PhysicalAxisConfig configure_axis(
      radiance3d::PhysicalAxisConfig axis) {
    axis.motion.homing.switch_normally_closed = false;
    axis.motion.microsteps = 1;
    axis.motion.maximum_speed_deg_per_s = 100.0;
    axis.motion.acceleration_deg_per_s2 = 200.0;
    axis.motion.motion_timeout_ms = 10000;
    return axis;
  }

  static radiance3d::PhysicalControllerConfig configure_controller(
      radiance3d::PhysicalControllerConfig value) {
    value.azimuth.axis = configure_axis(value.azimuth.axis);
    value.elevation.axis = configure_axis(value.elevation.axis);
    value.emergency_stop_debounce_ms = 1;
    return value;
  }

  void trust_positions() {
    azimuth.mutable_state().homed = true;
    azimuth.mutable_state().position_trusted = true;
    azimuth.mutable_state().trust_loss_reason =
        radiance3d::TrustLossReason::none;
    elevation.mutable_state().homed = true;
    elevation.mutable_state().position_trusted = true;
    elevation.mutable_state().trust_loss_reason =
        radiance3d::TrustLossReason::none;
  }

  void service_until_complete(std::uint32_t maximum = 2000) {
    for (std::uint32_t index = 0;
         index < maximum &&
         (azimuth.state().moving || elevation.state().moving);
         ++index) {
      platform.advance(100000);
      controller.service();
    }
  }
};

}  // namespace

void setUp() {}
void tearDown() {}

void test_provisional_gpio_is_valid_and_validation_rejects_conflicts() {
  auto config = radiance3d::provisional_esp32_dev_config();
  radiance3d::GpioValidationResult result =
      radiance3d::validate_esp32_gpio(config);
  TEST_ASSERT_TRUE(result.valid);
  TEST_ASSERT_EQUAL_INT64(0, result.bootstrapping_pin_mask);

  config.elevation.driver.step_pin = config.azimuth.driver.step_pin;
  result = radiance3d::validate_esp32_gpio(config);
  TEST_ASSERT_FALSE(result.valid);
  TEST_ASSERT_EQUAL_INT(config.azimuth.driver.step_pin,
                        result.duplicate_pin);

  config = radiance3d::provisional_esp32_dev_config();
  config.azimuth.driver.step_pin = 34;
  result = radiance3d::validate_esp32_gpio(config);
  TEST_ASSERT_FALSE(result.valid);
  TEST_ASSERT_EQUAL_INT(34, result.invalid_output_pin);
}

void test_safe_startup_initializes_both_axes_disabled_and_untrusted() {
  Fixture fixture;

  TEST_ASSERT_TRUE(fixture.controller.initialize());
  TEST_ASSERT_FALSE(fixture.azimuth_driver.enabled);
  TEST_ASSERT_FALSE(fixture.elevation_driver.enabled);
  TEST_ASSERT_FALSE(fixture.controller.state().azimuth.position_trusted);
  TEST_ASSERT_FALSE(fixture.controller.state().elevation.position_trusted);
}

void test_coordinated_move_completes_only_after_both_axes_finish() {
  Fixture fixture;
  TEST_ASSERT_TRUE(fixture.controller.initialize());
  fixture.trust_positions();

  TEST_ASSERT_TRUE(
      fixture.controller.move_absolute(18.0, 9.0, 20.0, 42).ok);
  TEST_ASSERT_TRUE(fixture.azimuth.state().moving);
  TEST_ASSERT_TRUE(fixture.elevation.state().moving);
  fixture.service_until_complete();

  TEST_ASSERT_FALSE(fixture.azimuth.state().moving);
  TEST_ASSERT_FALSE(fixture.elevation.state().moving);
  TEST_ASSERT_EQUAL_UINT32(42,
                           fixture.azimuth.state().last_completed_command);
  TEST_ASSERT_EQUAL_UINT32(42,
                           fixture.elevation.state().last_completed_command);
}

void test_one_axis_critical_fault_stops_coordinated_move_and_loses_trust() {
  Fixture fixture;
  TEST_ASSERT_TRUE(fixture.controller.initialize());
  fixture.trust_positions();
  TEST_ASSERT_TRUE(
      fixture.controller.move_absolute(90.0, 45.0, 10.0, 43).ok);
  fixture.elevation_driver.status.overtemperature_shutdown = true;
  fixture.elevation_driver.status.fault =
      radiance3d::DriverFault::overtemperature_shutdown;

  fixture.platform.advance(101000);
  fixture.controller.service();

  TEST_ASSERT_FALSE(fixture.azimuth.state().moving);
  TEST_ASSERT_FALSE(fixture.elevation.state().moving);
  TEST_ASSERT_FALSE(fixture.azimuth.state().position_trusted);
  TEST_ASSERT_FALSE(fixture.elevation.state().position_trusted);
}

void test_emergency_stop_latches_both_axes_and_requires_released_input() {
  Fixture fixture;
  TEST_ASSERT_TRUE(fixture.controller.initialize());
  fixture.trust_positions();
  TEST_ASSERT_TRUE(
      fixture.controller.move_absolute(90.0, 45.0, 10.0, 44).ok);

  fixture.platform.pins[fixture.config.emergency_stop_pin] = false;
  fixture.controller.service();
  fixture.platform.advance(2000);
  fixture.controller.service();

  TEST_ASSERT_TRUE(fixture.controller.state().emergency_stop_active);
  TEST_ASSERT_FALSE(fixture.azimuth_driver.enabled);
  TEST_ASSERT_FALSE(fixture.elevation_driver.enabled);
  TEST_ASSERT_FALSE(fixture.controller.clear_fault().ok);

  fixture.platform.pins[fixture.config.emergency_stop_pin] = true;
  fixture.controller.service();
  fixture.platform.advance(2000);
  fixture.controller.service();
  TEST_ASSERT_TRUE(fixture.controller.clear_fault().ok);
  TEST_ASSERT_FALSE(fixture.controller.state().emergency_stop_active);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_provisional_gpio_is_valid_and_validation_rejects_conflicts);
  RUN_TEST(test_safe_startup_initializes_both_axes_disabled_and_untrusted);
  RUN_TEST(test_coordinated_move_completes_only_after_both_axes_finish);
  RUN_TEST(test_one_axis_critical_fault_stops_coordinated_move_and_loses_trust);
  RUN_TEST(test_emergency_stop_latches_both_axes_and_requires_released_input);
  return UNITY_END();
}
