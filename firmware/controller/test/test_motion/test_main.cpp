#include <unity.h>

#include <string>

#include "motion_controller.hpp"
#include "protocol.hpp"

using radiance3d::AxisConfig;
using radiance3d::ProtocolEngine;

void setUp() {}
void tearDown() {}

void test_angular_conversion_is_derived_from_configuration() {
  AxisConfig config;
  config.motor_full_steps_per_revolution = 200;
  config.microsteps = 16;
  config.gear_ratio = 3.0;

  TEST_ASSERT_FLOAT_WITHIN(0.001f, 9600.0f,
                           static_cast<float>(config.steps_per_output_revolution()));
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0375f,
                           static_cast<float>(config.commanded_step_angle_deg()));
}

void test_motion_requires_homing_and_uses_configured_limits() {
  ProtocolEngine engine;

  TEST_ASSERT_TRUE(engine.handle("MOVE 0 0 5").find("ERR NOT_HOMED") == 0);
  TEST_ASSERT_EQUAL_STRING("OK HOME AXIS=BOTH", engine.handle("HOME BOTH").c_str());
  TEST_ASSERT_TRUE(engine.handle("SCAN_STEP 359 -90 5").find("OK SCAN_STEP") == 0);
  TEST_ASSERT_TRUE(engine.handle("MOVE 0 91 5").find("ERR LIMIT_REACHED") == 0);
}

void test_stop_invalidates_position_and_requires_rehoming() {
  ProtocolEngine engine;
  engine.handle("HOME BOTH");
  engine.handle("MOVE 1 1 5");

  TEST_ASSERT_EQUAL_STRING("OK STOP", engine.handle("STOP").c_str());
  TEST_ASSERT_EQUAL_STRING("OK CLEAR_FAULT", engine.handle("CLEAR_FAULT").c_str());
  TEST_ASSERT_TRUE(engine.handle("MOVE 2 2 5").find("ERR NOT_HOMED") == 0);
}

void test_driver_disable_invalidates_position_confidence() {
  ProtocolEngine engine;
  engine.handle("HOME BOTH");

  TEST_ASSERT_EQUAL_STRING("OK ENABLE VALUE=0", engine.handle("ENABLE 0").c_str());
  TEST_ASSERT_EQUAL_STRING("OK ENABLE VALUE=1", engine.handle("ENABLE 1").c_str());
  TEST_ASSERT_TRUE(engine.handle("MOVE 2 2 5").find("ERR NOT_HOMED") == 0);
}

void test_status_labels_position_as_commanded_and_untrusted_at_startup() {
  ProtocolEngine engine;
  const std::string status = engine.handle("STATUS");

  TEST_ASSERT_NOT_EQUAL(std::string::npos, status.find("POSITION_KIND=COMMANDED"));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, status.find("AZ_TRUSTED=0"));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, status.find("EL_TRUSTED=0"));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_angular_conversion_is_derived_from_configuration);
  RUN_TEST(test_motion_requires_homing_and_uses_configured_limits);
  RUN_TEST(test_stop_invalidates_position_and_requires_rehoming);
  RUN_TEST(test_driver_disable_invalidates_position_confidence);
  RUN_TEST(test_status_labels_position_as_commanded_and_untrusted_at_startup);
  return UNITY_END();
}
