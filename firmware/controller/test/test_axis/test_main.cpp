#include <unity.h>

#include <cmath>
#include <cstddef>
#include <cstdint>

#include "axis_controller.hpp"

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
  bool step_high{false};
  std::uint32_t rising_steps{0};
  radiance3d::DriverStatus status{};

  FakeDriver() {
    status.connected = true;
    status.standstill = true;
  }

  bool initialize() override { return connected; }
  radiance3d::DriverCapabilities capabilities() const override {
    return {true, true, true, true, true, true};
  }
  bool enable() override {
    enabled = connected;
    return enabled;
  }
  void disable() override {
    enabled = false;
    step_high = false;
  }
  bool set_direction(bool direction) override {
    positive_ = direction;
    return connected;
  }
  void set_step(bool high) override {
    if (high && !step_high) {
      ++rising_steps;
    }
    step_high = high;
  }
  bool set_current_milliamps(std::uint16_t value,
                             std::uint8_t hold) override {
    return connected && value > 0 && value <= 800 && hold <= 100;
  }
  bool set_microsteps(std::uint16_t value) override {
    return connected &&
           (value == 1 || value == 2 || value == 4 || value == 8 ||
            value == 16 || value == 32 || value == 64 || value == 128 ||
            value == 256);
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

 private:
  bool positive_{true};
};

radiance3d::PhysicalAxisConfig axis_config() {
  radiance3d::PhysicalAxisConfig config;
  config.name = "elevation";
  config.home_switch_pin = 32;
  config.motion.motor_full_steps_per_revolution = 200;
  config.motion.microsteps = 1;
  config.motion.motor_rms_current_ma = 400;
  config.motion.gear_ratio = 1.0;
  config.motion.minimum_angle_deg = -90.0;
  config.motion.maximum_angle_deg = 90.0;
  config.motion.home_offset_deg = 0.0;
  config.motion.maximum_speed_deg_per_s = 100.0;
  config.motion.acceleration_deg_per_s2 = 200.0;
  config.motion.motion_timeout_ms = 10000;
  config.motion.maximum_bench_test_steps = 20;
  config.motion.homing.switch_normally_closed = false;
  config.motion.homing.direction_negative = true;
  config.motion.homing.debounce_ms = 1;
  config.motion.homing.speed_deg_per_s = 50.0;
  config.motion.homing.slow_approach_deg_per_s = 10.0;
  config.motion.homing.backoff_deg = 3.6;
  config.motion.homing.timeout_ms = 5000;
  return config;
}

void service_until_stopped(radiance3d::AxisController& axis,
                           FakePlatform& platform,
                           std::uint32_t maximum_iterations = 1000) {
  for (std::uint32_t index = 0;
       index < maximum_iterations && axis.state().moving; ++index) {
    platform.advance(100000);
    axis.service();
  }
}

void settle_switch(radiance3d::AxisController& axis,
                   FakePlatform& platform, bool level_high) {
  platform.pins[32] = level_high;
  axis.service();
  platform.advance(2000);
  axis.service();
}

}  // namespace

void setUp() {}
void tearDown() {}

void test_conversions_use_integer_steps_and_half_away_from_zero_rounding() {
  FakePlatform platform;
  FakeDriver driver;
  radiance3d::AxisController axis(platform, driver, axis_config());
  TEST_ASSERT_TRUE(axis.initialize());

  std::int64_t steps = 0;
  TEST_ASSERT_TRUE(axis.degrees_to_steps(90.0, steps));
  TEST_ASSERT_EQUAL_INT64(50, steps);
  TEST_ASSERT_TRUE(axis.degrees_to_steps(-90.0, steps));
  TEST_ASSERT_EQUAL_INT64(-50, steps);
  TEST_ASSERT_TRUE(axis.degrees_to_steps(0.9, steps));
  TEST_ASSERT_EQUAL_INT64(1, steps);
  TEST_ASSERT_TRUE(axis.degrees_to_steps(-0.9, steps));
  TEST_ASSERT_EQUAL_INT64(-1, steps);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, -90.0f,
                           static_cast<float>(axis.steps_to_degrees(-50)));
  TEST_ASSERT_TRUE(axis.motor_full_steps_to_output_steps(10, steps));
  TEST_ASSERT_EQUAL_INT64(10, steps);
}

void test_absolute_and_relative_motion_require_homing() {
  FakePlatform platform;
  FakeDriver driver;
  radiance3d::AxisController axis(platform, driver, axis_config());
  TEST_ASSERT_TRUE(axis.initialize());

  TEST_ASSERT_EQUAL(radiance3d::FaultCode::not_homed,
                    axis.move_absolute_degrees(10.0, 10.0).fault);
  TEST_ASSERT_EQUAL(radiance3d::FaultCode::not_homed,
                    axis.move_relative_degrees(5.0, 10.0).fault);
}

void test_valid_motion_finishes_on_integer_target_and_limits_are_strict() {
  FakePlatform platform;
  FakeDriver driver;
  radiance3d::AxisController axis(platform, driver, axis_config());
  TEST_ASSERT_TRUE(axis.initialize());
  axis.mutable_state().position_trusted = true;
  axis.mutable_state().homed = true;

  TEST_ASSERT_TRUE(axis.move_absolute_degrees(18.0, 20.0, 7).ok);
  service_until_stopped(axis, platform);
  TEST_ASSERT_FALSE(axis.state().moving);
  TEST_ASSERT_EQUAL_INT64(10, axis.state().internal_step_position);
  TEST_ASSERT_EQUAL_UINT32(7, axis.state().last_completed_command);
  TEST_ASSERT_EQUAL(radiance3d::FaultCode::limit_reached,
                    axis.move_absolute_degrees(91.0, 20.0).fault);
  TEST_ASSERT_EQUAL(radiance3d::FaultCode::limit_reached,
                    axis.move_absolute_degrees(-91.0, 20.0).fault);
}

void test_stop_and_timeout_disable_motion_and_lose_trust() {
  FakePlatform platform;
  FakeDriver driver;
  auto config = axis_config();
  config.motion.motion_timeout_ms = 1;
  radiance3d::AxisController axis(platform, driver, config);
  TEST_ASSERT_TRUE(axis.initialize());
  axis.mutable_state().position_trusted = true;
  axis.mutable_state().homed = true;
  TEST_ASSERT_TRUE(axis.move_absolute_degrees(50.0, 10.0).ok);

  platform.advance(2000);
  axis.service();

  TEST_ASSERT_EQUAL(radiance3d::FaultCode::motion_timeout,
                    axis.state().fault);
  TEST_ASSERT_FALSE(axis.state().position_trusted);
  TEST_ASSERT_FALSE(axis.state().enabled);
}

void test_stuck_active_home_switch_fails_without_motion() {
  FakePlatform platform;
  platform.pins[32] = false;
  FakeDriver driver;
  radiance3d::AxisController axis(platform, driver, axis_config());
  TEST_ASSERT_TRUE(axis.initialize());
  settle_switch(axis, platform, false);

  const radiance3d::MotionResult result = axis.start_homing(10);

  TEST_ASSERT_FALSE(result.ok);
  TEST_ASSERT_EQUAL(radiance3d::FaultCode::homing_stuck_switch,
                    result.fault);
  TEST_ASSERT_FALSE(axis.state().enabled);
  TEST_ASSERT_FALSE(axis.state().position_trusted);
}

void test_homing_times_out_when_switch_never_activates() {
  FakePlatform platform;
  FakeDriver driver;
  auto config = axis_config();
  config.motion.homing.timeout_ms = 2;
  radiance3d::AxisController axis(platform, driver, config);
  TEST_ASSERT_TRUE(axis.initialize());
  TEST_ASSERT_TRUE(axis.start_homing(11).ok);

  platform.advance(3000);
  axis.service();

  TEST_ASSERT_EQUAL(radiance3d::FaultCode::homing_switch_never_triggered,
                    axis.state().fault);
  TEST_ASSERT_FALSE(axis.state().position_trusted);
}

void test_successful_two_pass_homing_applies_offset_and_trusts_position() {
  FakePlatform platform;
  FakeDriver driver;
  radiance3d::AxisController axis(platform, driver, axis_config());
  TEST_ASSERT_TRUE(axis.initialize());
  TEST_ASSERT_TRUE(axis.start_homing(12).ok);

  for (int index = 0; index < 6; ++index) {
    platform.advance(100000);
    axis.service();
  }
  settle_switch(axis, platform, false);
  TEST_ASSERT_EQUAL(radiance3d::HomingPhase::backoff,
                    axis.state().homing_phase);

  settle_switch(axis, platform, true);
  service_until_stopped(axis, platform);
  axis.service();
  TEST_ASSERT_EQUAL(radiance3d::HomingPhase::slow_approach,
                    axis.state().homing_phase);

  settle_switch(axis, platform, false);

  TEST_ASSERT_EQUAL(radiance3d::HomingPhase::complete,
                    axis.state().homing_phase);
  TEST_ASSERT_TRUE(axis.state().position_trusted);
  TEST_ASSERT_TRUE(axis.state().homed);
  TEST_ASSERT_EQUAL_INT64(0, axis.state().internal_step_position);
  TEST_ASSERT_EQUAL_UINT32(12, axis.state().last_completed_command);
}

void test_emergency_stop_during_homing_disables_and_loses_trust() {
  FakePlatform platform;
  FakeDriver driver;
  radiance3d::AxisController axis(platform, driver, axis_config());
  TEST_ASSERT_TRUE(axis.initialize());
  TEST_ASSERT_TRUE(axis.start_homing(13).ok);

  axis.emergency_stop();

  TEST_ASSERT_FALSE(axis.state().moving);
  TEST_ASSERT_FALSE(axis.state().enabled);
  TEST_ASSERT_FALSE(axis.state().position_trusted);
  TEST_ASSERT_EQUAL(radiance3d::FaultCode::emergency_stop,
                    axis.state().fault);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_conversions_use_integer_steps_and_half_away_from_zero_rounding);
  RUN_TEST(test_absolute_and_relative_motion_require_homing);
  RUN_TEST(test_valid_motion_finishes_on_integer_target_and_limits_are_strict);
  RUN_TEST(test_stop_and_timeout_disable_motion_and_lose_trust);
  RUN_TEST(test_stuck_active_home_switch_fails_without_motion);
  RUN_TEST(test_homing_times_out_when_switch_never_activates);
  RUN_TEST(test_successful_two_pass_homing_applies_offset_and_trusts_position);
  RUN_TEST(test_emergency_stop_during_homing_disables_and_loses_trust);
  return UNITY_END();
}
