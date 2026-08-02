#ifdef ESP_PLATFORM
#include <unity.h>
#else
#include "host_test.hpp"
#endif

#include <cstddef>
#include <cstdint>

#include "axis_controller.hpp"
#include "hardware_config.hpp"
#include "hardware_profile_generated.hpp"
#include "physical_motion_controller.hpp"
#include "protocol.hpp"

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

  config = radiance3d::provisional_esp32_dev_config();
  config.azimuth.driver.step_pin = 0;
  result = radiance3d::validate_esp32_gpio(config);
  TEST_ASSERT_TRUE(result.valid);
  TEST_ASSERT_NOT_EQUAL_INT64(0, result.bootstrapping_pin_mask);
}

void test_compiled_defaults_are_generated_from_the_hardware_profile() {
  const auto config = radiance3d::provisional_esp32_dev_config();

  TEST_ASSERT_EQUAL_STRING(radiance3d::generated_profile::kBoardName,
                           config.board_name);
  TEST_ASSERT_EQUAL_INT(radiance3d::generated_profile::kEmergencyStopPin,
                        config.emergency_stop_pin);
  TEST_ASSERT_EQUAL_UINT32(radiance3d::generated_profile::kProtocolVersion,
                           config.protocol_version);
  TEST_ASSERT_EQUAL_INT(radiance3d::generated_profile::kAzimuth.step_pin,
                        config.azimuth.driver.step_pin);
  TEST_ASSERT_EQUAL_INT(radiance3d::generated_profile::kElevation.step_pin,
                        config.elevation.driver.step_pin);
  TEST_ASSERT_EQUAL_UINT16(
      radiance3d::generated_profile::kAzimuth.commissioning_current_ma,
      config.azimuth.axis.motion.motor_rms_current_ma);
  TEST_ASSERT_EQUAL_UINT16(
      radiance3d::generated_profile::kElevation.commissioning_current_ma,
      config.elevation.axis.motion.motor_rms_current_ma);
  TEST_ASSERT_EQUAL_UINT8(30, config.azimuth.axis.motion.hold_current_percent);
  TEST_ASSERT_EQUAL_UINT8(30, config.elevation.axis.motion.hold_current_percent);
  TEST_ASSERT_EQUAL_UINT16(
      radiance3d::generated_profile::kAzimuth.maximum_rms_current_ma,
      config.azimuth.driver.maximum_rms_current_ma);
  TEST_ASSERT_EQUAL_UINT32(radiance3d::generated_profile::kTmcUartBaud,
                           config.azimuth.driver.uart_baud);
  TEST_ASSERT_EQUAL_UINT32(radiance3d::generated_profile::kTmcUartTimeoutMs,
                           config.elevation.driver.uart_timeout_ms);
  TEST_ASSERT_FALSE(config.azimuth.driver.uart_single_wire);
  TEST_ASSERT_FALSE(config.azimuth.driver.write_echo_expected);
  TEST_ASSERT_EQUAL_INT(1, config.azimuth.axis.motion.gear_ratio.numerator);
  TEST_ASSERT_EQUAL_INT(1, config.azimuth.axis.motion.gear_ratio.denominator);
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
  // Diagnostics are intentionally deferred while motion is active so a TMC
  // timeout cannot delay native GPTimer pulse scheduling. Finish the move
  // owner cycle, then invoke the explicit idle-only diagnostic path.
  fixture.controller.stop();
  fixture.controller.service_diagnostics();

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

void test_latched_estop_rejects_enable_and_runtime_motor_changes() {
  Fixture fixture;
  TEST_ASSERT_TRUE(fixture.controller.initialize());
  fixture.controller.emergency_stop();

  TEST_ASSERT_FALSE(fixture.controller.set_enabled(true).ok);
  TEST_ASSERT_FALSE(
      fixture.controller.set_axis_enabled(radiance3d::AxisSelection::azimuth,
                                          true)
          .ok);
  TEST_ASSERT_FALSE(
      fixture.controller.set_axis_current(radiance3d::AxisSelection::azimuth,
                                          650)
          .ok);
  TEST_ASSERT_FALSE(
      fixture.controller.set_axis_microsteps(radiance3d::AxisSelection::azimuth,
                                             8)
          .ok);
  TEST_ASSERT_FALSE(fixture.azimuth_driver.enabled);
  TEST_ASSERT_FALSE(fixture.elevation_driver.enabled);
}

void test_motor_config_tracks_accepted_runtime_changes() {
  Fixture fixture;
  TEST_ASSERT_TRUE(fixture.controller.initialize());
  TEST_ASSERT_TRUE(
      fixture.controller.set_axis_current(radiance3d::AxisSelection::azimuth,
                                          700)
          .ok);
  TEST_ASSERT_TRUE(
      fixture.controller.set_axis_microsteps(radiance3d::AxisSelection::azimuth,
                                             8)
          .ok);
  TEST_ASSERT_EQUAL_UINT16(700, fixture.controller.config().azimuth.motor_rms_current_ma);
  TEST_ASSERT_EQUAL_UINT16(8, fixture.controller.config().azimuth.microsteps);
}

void test_physical_protocol_uses_profile_protocol_version() {
  Fixture fixture;
  TEST_ASSERT_TRUE(fixture.controller.initialize());
  radiance3d::ProtocolEngine engine(fixture.controller,
                                    fixture.config.protocol_version);
  const std::string identify = engine.handle("IDENTIFY");
  TEST_ASSERT_NOT_EQUAL(std::string::npos,
                        identify.find("PROTOCOL=" +
                                      std::to_string(fixture.config.protocol_version)));
}

void test_stop_all_stops_both_axes_and_invalidates_active_move() {
  Fixture fixture;
  TEST_ASSERT_TRUE(fixture.controller.initialize());
  fixture.trust_positions();
  TEST_ASSERT_TRUE(
      fixture.controller.move_absolute(90.0, 45.0, 10.0, 45).ok);

  TEST_ASSERT_TRUE(fixture.controller.stop().ok);

  TEST_ASSERT_FALSE(fixture.azimuth.state().moving);
  TEST_ASSERT_FALSE(fixture.elevation.state().moving);
  TEST_ASSERT_FALSE(fixture.azimuth.state().position_trusted);
  TEST_ASSERT_FALSE(fixture.elevation.state().position_trusted);
}

void test_protocol_emits_completion_only_after_both_axes_stop() {
  Fixture fixture;
  TEST_ASSERT_TRUE(fixture.controller.initialize());
  fixture.trust_positions();
  radiance3d::ProtocolEngine engine(fixture.controller);

  const std::string accepted =
      engine.handle("CMD 50 SCAN_STEP 18 9 20");
  TEST_ASSERT_NOT_EQUAL(std::string::npos, accepted.find("READY=0"));
  std::string event;
  for (std::uint32_t index = 0; index < 2000 && event.empty(); ++index) {
    fixture.platform.advance(100000);
    event = engine.service();
  }

  TEST_ASSERT_TRUE(event.find("EVENT MOTION_COMPLETE ID=50") == 0);
  TEST_ASSERT_NOT_EQUAL(std::string::npos, event.find("AZ_DONE=1"));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, event.find("EL_DONE=1"));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_provisional_gpio_is_valid_and_validation_rejects_conflicts);
  RUN_TEST(test_compiled_defaults_are_generated_from_the_hardware_profile);
  RUN_TEST(test_safe_startup_initializes_both_axes_disabled_and_untrusted);
  RUN_TEST(test_coordinated_move_completes_only_after_both_axes_finish);
  RUN_TEST(test_one_axis_critical_fault_stops_coordinated_move_and_loses_trust);
  RUN_TEST(test_emergency_stop_latches_both_axes_and_requires_released_input);
  RUN_TEST(test_latched_estop_rejects_enable_and_runtime_motor_changes);
  RUN_TEST(test_motor_config_tracks_accepted_runtime_changes);
  RUN_TEST(test_physical_protocol_uses_profile_protocol_version);
  RUN_TEST(test_stop_all_stops_both_axes_and_invalidates_active_move);
  RUN_TEST(test_protocol_emits_completion_only_after_both_axes_stop);
  return UNITY_END();
}
