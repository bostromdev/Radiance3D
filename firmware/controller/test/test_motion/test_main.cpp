#include <unity.h>

#include <string>

#include "motion_controller.hpp"
#include "protocol.hpp"

using radiance3d::AxisConfig;
using radiance3d::ProtocolEngine;
using radiance3d::SimulatedMotionController;

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

void test_correlated_commands_reject_duplicate_and_stale_ids() {
  ProtocolEngine engine;

  TEST_ASSERT_TRUE(engine.handle("CMD 2 STATUS").find("OK ID=2 STATUS") == 0);
  TEST_ASSERT_TRUE(
      engine.handle("CMD 2 STATUS").find("ERR ID=2 DUPLICATE_COMMAND") == 0);
  TEST_ASSERT_TRUE(
      engine.handle("CMD 1 STATUS").find("ERR ID=1 STALE_COMMAND") == 0);
}

void test_heartbeat_is_correlated_and_rejects_arguments() {
  ProtocolEngine engine;

  TEST_ASSERT_EQUAL_STRING("OK ID=1 HEARTBEAT",
                           engine.handle("CMD 1 HEARTBEAT").c_str());
  TEST_ASSERT_TRUE(engine.handle("CMD 2 HEARTBEAT extra")
                       .find("ERR ID=2 INVALID_ARGUMENT") == 0);
}

void test_bench_commands_are_relative_and_explicitly_untrusted() {
  ProtocolEngine engine;

  const std::string response = engine.handle("CMD 1 MOTOR STEP AZ 10");

  TEST_ASSERT_TRUE(response.find("OK ID=1 MOTOR_STEP AXIS=AZ") == 0);
  TEST_ASSERT_NOT_EQUAL(std::string::npos,
                        response.find("POSITION_TRUSTED=0"));
  TEST_ASSERT_TRUE(
      engine.handle("CMD 2 MOTOR SET_CURRENT AZ 0")
              .find("ERR ID=2 INVALID_ARGUMENT") == 0);
  TEST_ASSERT_TRUE(
      engine.handle("CMD 3 MOTOR SET_MICROSTEPS EL 3")
              .find("ERR ID=3 INVALID_ARGUMENT") == 0);
}

void test_malformed_motor_command_and_unsupported_axis_are_rejected() {
  ProtocolEngine engine;

  TEST_ASSERT_TRUE(engine.handle("MOTOR STEP Z 10")
                       .find("ERR INVALID_ARGUMENT") == 0);
  TEST_ASSERT_TRUE(engine.handle("MOTOR STEP AZ")
                       .find("ERR INVALID_ARGUMENT") == 0);
}

void test_protocol_exposes_transport_neutral_axis_configuration() {
  ProtocolEngine engine;
  const std::string response = engine.handle("CMD 1 MOTOR CONFIG AZ");

  TEST_ASSERT_TRUE(response.find("OK ID=1 MOTOR_CONFIG AXIS=AZ") == 0);
  TEST_ASSERT_NOT_EQUAL(std::string::npos, response.find("FULL_STEPS=200"));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, response.find("MICROSTEPS=16"));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, response.find("RMS_MA=400"));
}

void test_simulator_models_missing_driver_and_critical_thermal_fault() {
  SimulatedMotionController controller(
      radiance3d::provisional_simulator_config());
  TEST_ASSERT_TRUE(controller.initialize());
  controller.home(radiance3d::AxisSelection::both);
  radiance3d::DriverStatus missing;
  missing.connected = false;
  missing.fault = radiance3d::DriverFault::communication_failure;
  controller.simulate_driver_status(radiance3d::AxisSelection::azimuth,
                                    missing);
  controller.service();

  TEST_ASSERT_FALSE(controller.state().azimuth.position_trusted);
  TEST_ASSERT_EQUAL(radiance3d::FaultCode::driver_communication,
                    controller.state().azimuth.fault);

  radiance3d::DriverStatus thermal;
  thermal.connected = true;
  thermal.overtemperature_shutdown = true;
  thermal.fault = radiance3d::DriverFault::overtemperature_shutdown;
  controller.simulate_driver_status(radiance3d::AxisSelection::elevation,
                                    thermal);
  controller.service();
  TEST_ASSERT_FALSE(controller.state().elevation.enabled);
  TEST_ASSERT_EQUAL(radiance3d::FaultCode::driver_critical,
                    controller.state().elevation.fault);
}

void test_simulator_models_homing_failure_and_reset_trust_loss() {
  SimulatedMotionController controller(
      radiance3d::provisional_simulator_config());
  controller.simulate_homing_failure(
      radiance3d::AxisSelection::azimuth,
      radiance3d::FaultCode::homing_switch_never_triggered);

  const radiance3d::MotionResult home =
      controller.home(radiance3d::AxisSelection::azimuth);
  TEST_ASSERT_FALSE(home.ok);
  TEST_ASSERT_EQUAL(
      radiance3d::FaultCode::homing_switch_never_triggered, home.fault);

  controller.simulate_homing_failure(radiance3d::AxisSelection::azimuth,
                                     radiance3d::FaultCode::none);
  controller.clear_fault();
  controller.home(radiance3d::AxisSelection::both);
  controller.simulate_reset(radiance3d::AxisSelection::both);
  TEST_ASSERT_FALSE(controller.state().azimuth.position_trusted);
  TEST_ASSERT_EQUAL(radiance3d::TrustLossReason::watchdog_reset_during_motion,
                    controller.state().azimuth.trust_loss_reason);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_angular_conversion_is_derived_from_configuration);
  RUN_TEST(test_motion_requires_homing_and_uses_configured_limits);
  RUN_TEST(test_stop_invalidates_position_and_requires_rehoming);
  RUN_TEST(test_driver_disable_invalidates_position_confidence);
  RUN_TEST(test_status_labels_position_as_commanded_and_untrusted_at_startup);
  RUN_TEST(test_correlated_commands_reject_duplicate_and_stale_ids);
  RUN_TEST(test_heartbeat_is_correlated_and_rejects_arguments);
  RUN_TEST(test_bench_commands_are_relative_and_explicitly_untrusted);
  RUN_TEST(test_malformed_motor_command_and_unsupported_axis_are_rejected);
  RUN_TEST(test_protocol_exposes_transport_neutral_axis_configuration);
  RUN_TEST(test_simulator_models_missing_driver_and_critical_thermal_fault);
  RUN_TEST(test_simulator_models_homing_failure_and_reset_trust_loss);
  return UNITY_END();
}
