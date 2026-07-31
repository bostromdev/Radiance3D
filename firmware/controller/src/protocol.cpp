#include "protocol.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>

namespace radiance3d {
namespace {

const char* fault_name(const FaultCode code) {
  switch (code) {
    case FaultCode::none:
      return "NONE";
    case FaultCode::invalid_command:
      return "INVALID_COMMAND";
    case FaultCode::invalid_argument:
      return "INVALID_ARGUMENT";
    case FaultCode::invalid_configuration:
      return "INVALID_CONFIGURATION";
    case FaultCode::not_homed:
      return "NOT_HOMED";
    case FaultCode::position_untrusted:
      return "POSITION_UNTRUSTED";
    case FaultCode::limit_reached:
      return "LIMIT_REACHED";
    case FaultCode::motion_timeout:
      return "MOTION_TIMEOUT";
    case FaultCode::driver_disabled:
      return "DRIVER_DISABLED";
    case FaultCode::emergency_stop:
      return "EMERGENCY_STOP";
    case FaultCode::driver_communication:
      return "DRIVER_COMMUNICATION";
    case FaultCode::driver_critical:
      return "DRIVER_CRITICAL";
    case FaultCode::homing_stuck_switch:
      return "HOMING_STUCK_SWITCH";
    case FaultCode::homing_switch_never_triggered:
      return "HOMING_SWITCH_NEVER_TRIGGERED";
    case FaultCode::homing_switch_failed_release:
      return "HOMING_SWITCH_FAILED_RELEASE";
    case FaultCode::homing_timeout:
      return "HOMING_TIMEOUT";
    case FaultCode::unexpected_home_switch:
      return "UNEXPECTED_HOME_SWITCH";
    case FaultCode::stopped:
      return "STOPPED";
  }
  return "UNKNOWN";
}

const char* trust_reason_name(const TrustLossReason reason) {
  switch (reason) {
    case TrustLossReason::startup:
      return "STARTUP";
    case TrustLossReason::none:
      return "NONE";
    case TrustLossReason::reset:
      return "RESET";
    case TrustLossReason::power_loss:
      return "POWER_LOSS";
    case TrustLossReason::emergency_stop:
      return "EMERGENCY_STOP";
    case TrustLossReason::driver_fault:
      return "DRIVER_FAULT";
    case TrustLossReason::driver_disabled_during_motion:
      return "DRIVER_DISABLED";
    case TrustLossReason::motion_timeout:
      return "MOTION_TIMEOUT";
    case TrustLossReason::suspected_missed_step:
      return "SUSPECTED_MISSED_STEP";
    case TrustLossReason::manual_movement:
      return "MANUAL_MOVEMENT";
    case TrustLossReason::configuration_changed:
      return "CONFIGURATION_CHANGED";
    case TrustLossReason::homing_failed:
      return "HOMING_FAILED";
    case TrustLossReason::watchdog_reset_during_motion:
      return "WATCHDOG_RESET";
    case TrustLossReason::stopped:
      return "STOPPED";
  }
  return "UNKNOWN";
}

const char* homing_phase_name(const HomingPhase phase) {
  switch (phase) {
    case HomingPhase::idle:
      return "IDLE";
    case HomingPhase::validate_switch:
      return "VALIDATE_SWITCH";
    case HomingPhase::fast_approach:
      return "FAST_APPROACH";
    case HomingPhase::backoff:
      return "BACKOFF";
    case HomingPhase::confirm_release:
      return "CONFIRM_RELEASE";
    case HomingPhase::slow_approach:
      return "SLOW_APPROACH";
    case HomingPhase::apply_offset:
      return "APPLY_OFFSET";
    case HomingPhase::complete:
      return "COMPLETE";
    case HomingPhase::failed:
      return "FAILED";
  }
  return "UNKNOWN";
}

const char* driver_fault_name(const DriverFault fault) {
  switch (fault) {
    case DriverFault::none:
      return "NONE";
    case DriverFault::invalid_configuration:
      return "INVALID_CONFIGURATION";
    case DriverFault::communication_failure:
      return "COMMUNICATION_FAILURE";
    case DriverFault::reset_detected:
      return "RESET_DETECTED";
    case DriverFault::undervoltage:
      return "UNDERVOLTAGE";
    case DriverFault::overtemperature_warning:
      return "OVERTEMPERATURE_WARNING";
    case DriverFault::overtemperature_shutdown:
      return "OVERTEMPERATURE_SHUTDOWN";
    case DriverFault::short_to_ground:
      return "SHORT_TO_GROUND";
    case DriverFault::short_to_supply:
      return "SHORT_TO_SUPPLY";
    case DriverFault::open_load:
      return "OPEN_LOAD";
  }
  return "UNKNOWN";
}

bool read_double(std::istringstream& input, double& value) {
  input >> value;
  return !input.fail() && std::isfinite(value);
}

bool no_extra_arguments(std::istringstream& input) {
  std::string extra;
  input >> extra;
  return extra.empty();
}

bool parse_axis(const std::string& text, AxisSelection& axis,
                const bool allow_both = true) {
  if (text == "AZ") {
    axis = AxisSelection::azimuth;
    return true;
  }
  if (text == "EL") {
    axis = AxisSelection::elevation;
    return true;
  }
  if (allow_both && text == "BOTH") {
    axis = AxisSelection::both;
    return true;
  }
  return false;
}

std::string with_command_id(const std::string& response,
                            const std::uint32_t command_id) {
  if (command_id == 0) {
    return response;
  }
  const std::string id = " ID=" + std::to_string(command_id);
  if (response.rfind("OK", 0) == 0) {
    return "OK" + id + response.substr(2);
  }
  if (response.rfind("ERR", 0) == 0) {
    return "ERR" + id + response.substr(3);
  }
  return response + id;
}

}  // namespace

ProtocolEngine::ProtocolEngine()
    : default_controller_(provisional_simulator_config()),
      controller_(&default_controller_) {}

ProtocolEngine::ProtocolEngine(MotionController& controller,
                               const std::uint32_t protocol_version)
    : default_controller_(provisional_simulator_config()),
      controller_(&controller),
      protocol_version_(protocol_version) {}

const ControllerState& ProtocolEngine::state() const {
  return controller_->state();
}

std::string ProtocolEngine::fault(const FaultCode code,
                                  const std::string& detail) {
  controller_->report_fault(code);
  return "ERR " + std::string(fault_name(code)) + " " + detail;
}

std::string ProtocolEngine::axis_status(const AxisSelection axis) const {
  const AxisState& selected =
      axis == AxisSelection::elevation ? state().elevation : state().azimuth;
  const char* axis_name =
      axis == AxisSelection::elevation ? "EL" : "AZ";
  std::ostringstream output;
  output << std::fixed << std::setprecision(3) << "AXIS=" << axis_name
         << " DEG=" << selected.commanded_position_deg
         << " STEPS=" << selected.internal_step_position
         << " TARGET_STEPS=" << selected.target_step_position
         << " POSITION_KIND=COMMANDED"
         << " TRUSTED=" << (selected.position_trusted ? 1 : 0)
         << " TRUST_LOSS=" << trust_reason_name(selected.trust_loss_reason)
         << " HOMED=" << (selected.homed ? 1 : 0)
         << " MOVING=" << (selected.moving ? 1 : 0)
         << " ENABLED=" << (selected.enabled ? 1 : 0)
         << " HOME_ACTIVE=" << (selected.home_switch_active ? 1 : 0)
         << " HOMING=" << homing_phase_name(selected.homing_phase)
         << " FAULT=" << fault_name(selected.fault)
         << " LAST_COMMAND=" << selected.last_completed_command;
  return output.str();
}

std::string ProtocolEngine::status() const {
  std::ostringstream output;
  output << std::fixed << std::setprecision(3) << "OK STATUS"
         << " AZ_DEG=" << state().azimuth.commanded_position_deg
         << " EL_DEG=" << state().elevation.commanded_position_deg
         << " POSITION_KIND=COMMANDED"
         << " AZ_HOMED=" << (state().azimuth.homed ? 1 : 0)
         << " EL_HOMED=" << (state().elevation.homed ? 1 : 0)
         << " AZ_TRUSTED="
         << (state().azimuth.position_trusted ? 1 : 0)
         << " EL_TRUSTED="
         << (state().elevation.position_trusted ? 1 : 0)
         << " DRIVERS_ENABLED="
         << ((state().azimuth.enabled && state().elevation.enabled) ? 1 : 0)
         << " " << axis_status(AxisSelection::azimuth) << " "
         << axis_status(AxisSelection::elevation)
         << " STOPPED=" << (state().stopped ? 1 : 0)
         << " ESTOP=" << (state().emergency_stop_active ? 1 : 0)
         << " FAULT=" << fault_name(state().fault);
  return output.str();
}

std::string ProtocolEngine::diagnostics(const AxisSelection axis) const {
  const DriverStatus driver = controller_->driver_status(axis);
  const DriverCapabilities capabilities =
      controller_->driver_capabilities(axis);
  std::ostringstream output;
  output << "OK MOTOR_DIAGNOSTICS AXIS="
         << (axis == AxisSelection::azimuth ? "AZ" : "EL")
         << " CONNECTED=" << (driver.connected ? 1 : 0)
         << " ENABLED=" << (driver.enabled ? 1 : 0)
         << " FAULT=" << driver_fault_name(driver.fault)
         << " OTPW=" << (driver.overtemperature_warning ? 1 : 0)
         << " OT=" << (driver.overtemperature_shutdown ? 1 : 0)
         << " UV=" << (driver.undervoltage ? 1 : 0)
         << " RESET=" << (driver.reset_detected ? 1 : 0)
         << " S2GA=" << (driver.short_to_ground_a ? 1 : 0)
         << " S2GB=" << (driver.short_to_ground_b ? 1 : 0)
         << " S2VSA=" << (driver.short_to_supply_a ? 1 : 0)
         << " S2VSB=" << (driver.short_to_supply_b ? 1 : 0)
         << " OLA=" << (driver.open_load_a ? 1 : 0)
         << " OLB=" << (driver.open_load_b ? 1 : 0)
         << " CURRENT_SCALE=" << static_cast<unsigned>(driver.current_scale)
         << " CAP_UART=" << (capabilities.uart_diagnostics ? 1 : 0)
         << " CAP_CURRENT=" << (capabilities.configurable_current ? 1 : 0)
         << " CAP_MICROSTEPS="
         << (capabilities.configurable_microsteps ? 1 : 0);
  return output.str();
}

std::string ProtocolEngine::handle(const std::string& line) {
  std::istringstream input(line);
  std::string first;
  input >> first;
  if (first != "CMD") {
    return handle_command(line, 0);
  }

  std::uint64_t parsed_id = 0;
  input >> parsed_id;
  std::string command;
  std::getline(input, command);
  const std::size_t first_non_space = command.find_first_not_of(" \t");
  if (input.fail() || parsed_id == 0 ||
      parsed_id > std::numeric_limits<std::uint32_t>::max() ||
      first_non_space == std::string::npos) {
    return "ERR INVALID_ARGUMENT CMD expects positive-id and command";
  }
  const std::uint32_t command_id = static_cast<std::uint32_t>(parsed_id);
  if (command_id == last_command_id_) {
    return "ERR ID=" + std::to_string(command_id) +
           " DUPLICATE_COMMAND command id was already consumed";
  }
  if (command_id < last_command_id_) {
    return "ERR ID=" + std::to_string(command_id) +
           " STALE_COMMAND command id is older than last consumed id";
  }
  last_command_id_ = command_id;
  return with_command_id(
      handle_command(command.substr(first_non_space), command_id),
      command_id);
}

std::string ProtocolEngine::handle_command(const std::string& line,
                                           const std::uint32_t command_id) {
  std::istringstream input(line);
  std::string command;
  input >> command;

  if (command == "IDENTIFY") {
    if (!no_extra_arguments(input)) {
      return fault(FaultCode::invalid_argument,
                   "IDENTIFY expects no arguments");
    }
    return "OK IDENTIFY DEVICE=Radiance3D CONTROLLER=motion "
           "PROTOCOL=" +
           std::to_string(protocol_version_) +
           " MODE=" + (controller_ == &default_controller_ ? "SIMULATOR"
                                                            : "PHYSICAL");
  }
  if (command == "STATUS" || command == "POSITION") {
    if (!no_extra_arguments(input)) {
      return fault(FaultCode::invalid_argument,
                   command + " expects no arguments");
    }
    return status();
  }
  if (command == "HEARTBEAT") {
    if (!no_extra_arguments(input)) {
      return fault(FaultCode::invalid_argument,
                   "HEARTBEAT expects no arguments");
    }
    return "OK HEARTBEAT";
  }
  if (command == "CLEAR_FAULT" || command == "RESET_ESTOP") {
    if (!no_extra_arguments(input)) {
      return fault(FaultCode::invalid_argument,
                   command + " expects no arguments");
    }
    const MotionResult result = controller_->clear_fault();
    if (!result.ok) {
      return fault(result.fault,
                   "physical emergency-stop input or driver fault remains active");
    }
    return "OK " + command;
  }
  if (command == "STOP") {
    if (!no_extra_arguments(input)) {
      return fault(FaultCode::invalid_argument,
                   "STOP expects no arguments");
    }
    controller_->stop();
    return "OK STOP";
  }
  if (command == "E_STOP") {
    if (!no_extra_arguments(input)) {
      return fault(FaultCode::invalid_argument,
                   "E_STOP expects no arguments");
    }
    controller_->emergency_stop();
    return "OK E_STOP";
  }
  if (command == "ENABLE") {
    int enabled = -1;
    input >> enabled;
    if (input.fail() || (enabled != 0 && enabled != 1) ||
        !no_extra_arguments(input)) {
      return fault(FaultCode::invalid_argument,
                   "ENABLE expects 0 or 1");
    }
    const MotionResult result = controller_->set_enabled(enabled == 1);
    if (!result.ok && enabled == 1) {
      return fault(result.fault, "driver state change failed");
    }
    return "OK ENABLE VALUE=" + std::to_string(enabled);
  }
  if (command == "HOME") {
    std::string axis_text;
    AxisSelection axis = AxisSelection::both;
    input >> axis_text;
    if (!parse_axis(axis_text, axis) || !no_extra_arguments(input)) {
      return fault(FaultCode::invalid_argument,
                   "HOME expects AZ, EL, or BOTH");
    }
    const MotionResult result = controller_->home(axis, command_id);
    if (!result.ok) {
      return fault(result.fault,
                   "homing rejected by motion controller");
    }
    return "OK HOME AXIS=" + axis_text +
           (state().azimuth.moving || state().elevation.moving
                ? " ACCEPTED=1 READY=0"
                : "");
  }
  if (command == "MOVE" || command == "SCAN_STEP") {
    double azimuth = 0.0;
    double elevation = 0.0;
    double speed = 0.0;
    if (!read_double(input, azimuth) || !read_double(input, elevation) ||
        !read_double(input, speed) || speed <= 0.0 ||
        !no_extra_arguments(input)) {
      return fault(FaultCode::invalid_argument,
                   command + " expects AZ_DEG EL_DEG DEG_PER_S");
    }
    const MotionResult result = controller_->move_absolute(
        azimuth, elevation, speed, command_id);
    if (!result.ok) {
      return fault(
          result.fault,
          "motion rejected by configured controller limits or state");
    }
    const bool moving = state().azimuth.moving || state().elevation.moving;
    std::ostringstream output;
    output << std::fixed << std::setprecision(3) << "OK " << command
           << " AZ_DEG=" << azimuth << " EL_DEG=" << elevation
           << " DEG_PER_S=" << speed;
    if (command == "SCAN_STEP") {
      output << " READY=" << (moving ? 0 : 1)
             << " POSITION_KIND=COMMANDED";
    } else if (moving) {
      output << " ACCEPTED=1";
    }
    return output.str();
  }
  if (command == "MOVE_REL") {
    std::string axis_text;
    AxisSelection axis = AxisSelection::both;
    double delta = 0.0;
    double speed = 0.0;
    input >> axis_text;
    if (!parse_axis(axis_text, axis) || !read_double(input, delta) ||
        !read_double(input, speed) || speed <= 0.0 ||
        !no_extra_arguments(input)) {
      return fault(FaultCode::invalid_argument,
                   "MOVE_REL expects AXIS DELTA_DEG DEG_PER_S");
    }
    const MotionResult result =
        controller_->move_relative(axis, delta, speed, command_id);
    if (!result.ok) {
      return fault(result.fault, "relative motion rejected");
    }
    return "OK MOVE_REL AXIS=" + axis_text + " ACCEPTED=1";
  }
  if (command == "MOTOR") {
    std::string operation;
    input >> operation;
    if (operation == "IDENTIFY" && no_extra_arguments(input)) {
      const DriverCapabilities azimuth =
          controller_->driver_capabilities(AxisSelection::azimuth);
      const DriverCapabilities elevation =
          controller_->driver_capabilities(AxisSelection::elevation);
      return "OK MOTOR_IDENTIFY AZ_PRESENT=" +
             std::to_string(
                 controller_->driver_status(AxisSelection::azimuth).connected) +
             " EL_PRESENT=" +
             std::to_string(controller_
                                ->driver_status(AxisSelection::elevation)
                                .connected) +
             " UART=" +
             std::to_string(azimuth.uart_diagnostics &&
                            elevation.uart_diagnostics);
    }

    std::string axis_text;
    AxisSelection axis = AxisSelection::both;
    input >> axis_text;
    if (!parse_axis(axis_text, axis, false)) {
      return fault(FaultCode::invalid_argument,
                   "MOTOR operation expects AZ or EL");
    }
    if (operation == "STATUS" && no_extra_arguments(input)) {
      return "OK MOTOR_STATUS " + axis_status(axis);
    }
    if (operation == "CONFIG" && no_extra_arguments(input)) {
      const AxisConfig& config =
          axis == AxisSelection::azimuth ? controller_->config().azimuth
                                         : controller_->config().elevation;
      std::ostringstream output;
      output << std::fixed << std::setprecision(3)
             << "OK MOTOR_CONFIG AXIS=" << axis_text
             << " FULL_STEPS=" << config.motor_full_steps_per_revolution
             << " MICROSTEPS=" << config.microsteps
             << " GEAR_RATIO=" << config.gear_ratio.as_double()
             << " MIN_DEG=" << config.minimum_angle_deg
             << " MAX_DEG=" << config.maximum_angle_deg
             << " MAX_SPEED=" << config.maximum_speed_deg_per_s
             << " ACCEL=" << config.acceleration_deg_per_s2
             << " RMS_MA=" << config.motor_rms_current_ma
             << " HOLD_PERCENT="
             << static_cast<unsigned>(config.hold_current_percent)
             << " HOME_OFFSET=" << config.home_offset_deg;
      return output.str();
    }
    if (operation == "DIAGNOSTICS" && no_extra_arguments(input)) {
      return diagnostics(axis);
    }
    if ((operation == "ENABLE" || operation == "DISABLE") &&
        no_extra_arguments(input)) {
      const bool enabled = operation == "ENABLE";
      const MotionResult result =
          controller_->set_axis_enabled(axis, enabled);
      if (!result.ok) {
        return fault(result.fault, "axis enable change rejected");
      }
      return "OK MOTOR_" + operation + " AXIS=" + axis_text;
    }
    if (operation == "STOP" && no_extra_arguments(input)) {
      const MotionResult result = controller_->stop_axis(axis);
      return result.ok ? "OK MOTOR_STOP AXIS=" + axis_text
                       : fault(result.fault, "axis stop rejected");
    }
    if (operation == "STEP") {
      long long signed_steps = 0;
      input >> signed_steps;
      if (input.fail() || !no_extra_arguments(input)) {
        return fault(FaultCode::invalid_argument,
                     "MOTOR STEP expects AXIS SIGNED_STEPS");
      }
      const MotionResult result = controller_->bench_move_steps(
          axis, static_cast<std::int64_t>(signed_steps), command_id);
      return result.ok
                 ? "OK MOTOR_STEP AXIS=" + axis_text +
                       " ACCEPTED=1 POSITION_TRUSTED=0"
                 : fault(result.fault, "bench step rejected");
    }
    if (operation == "MOVE_DEGREES") {
      double degrees = 0.0;
      if (!read_double(input, degrees) || !no_extra_arguments(input)) {
        return fault(FaultCode::invalid_argument,
                     "MOTOR MOVE_DEGREES expects AXIS SIGNED_DEGREES");
      }
      const AxisConfig& config =
          axis == AxisSelection::azimuth ? controller_->config().azimuth
                                         : controller_->config().elevation;
      const long double scaled =
          static_cast<long double>(degrees) *
          config.steps_per_output_revolution() / 360.0L;
      if (scaled >
              static_cast<long double>(
                  std::numeric_limits<std::int64_t>::max()) ||
          scaled <
              static_cast<long double>(
                  std::numeric_limits<std::int64_t>::min())) {
        return fault(FaultCode::invalid_argument,
                     "bench angle overflows step range");
      }
      const MotionResult result = controller_->bench_move_steps(
          axis, static_cast<std::int64_t>(std::llround(scaled)),
          command_id);
      return result.ok
                 ? "OK MOTOR_MOVE_DEGREES AXIS=" + axis_text +
                       " ACCEPTED=1 POSITION_TRUSTED=0"
                 : fault(result.fault, "bench angle rejected");
    }
    if (operation == "SET_CURRENT") {
      unsigned long current = 0;
      input >> current;
      if (input.fail() ||
          current > std::numeric_limits<std::uint16_t>::max() ||
          !no_extra_arguments(input)) {
        return fault(FaultCode::invalid_argument,
                     "MOTOR SET_CURRENT expects AXIS RMS_MA");
      }
      const MotionResult result = controller_->set_axis_current(
          axis, static_cast<std::uint16_t>(current));
      return result.ok
                 ? "OK MOTOR_SET_CURRENT AXIS=" + axis_text +
                       " RMS_MA=" + std::to_string(current)
                 : fault(result.fault, "current rejected");
    }
    if (operation == "SET_MICROSTEPS") {
      unsigned long microsteps = 0;
      input >> microsteps;
      if (input.fail() ||
          microsteps > std::numeric_limits<std::uint16_t>::max() ||
          !no_extra_arguments(input)) {
        return fault(FaultCode::invalid_argument,
                     "MOTOR SET_MICROSTEPS expects AXIS VALUE");
      }
      const MotionResult result = controller_->set_axis_microsteps(
          axis, static_cast<std::uint16_t>(microsteps));
      return result.ok
                 ? "OK MOTOR_SET_MICROSTEPS AXIS=" + axis_text +
                       " VALUE=" + std::to_string(microsteps) +
                       " POSITION_TRUSTED=0"
                 : fault(result.fault, "microsteps rejected");
    }
    return fault(FaultCode::invalid_command,
                 "unknown or malformed MOTOR operation");
  }

  return fault(FaultCode::invalid_command, "unknown command");
}

std::string ProtocolEngine::service() {
  controller_->service();
  const ControllerState& current = state();
  std::string event;
  if (current.emergency_stop_active != previous_estop_) {
    event = "EVENT ESTOP ACTIVE=" +
            std::to_string(current.emergency_stop_active ? 1 : 0);
  } else if (current.fault != previous_fault_) {
    event = "EVENT FAULT CODE=" +
            std::string(fault_name(current.fault));
  } else {
    const bool completed =
        (previous_azimuth_moving_ || previous_elevation_moving_) &&
        !current.azimuth.moving && !current.elevation.moving;
    if (completed && current.fault == FaultCode::none) {
      const std::uint32_t completed_id =
          std::max(current.azimuth.last_completed_command,
                   current.elevation.last_completed_command);
      event = "EVENT MOTION_COMPLETE ID=" +
              std::to_string(completed_id) +
              " AZ_DONE=" +
              std::to_string(current.azimuth.moving ? 0 : 1) +
              " EL_DONE=" +
              std::to_string(current.elevation.moving ? 0 : 1);
    }
  }
  previous_estop_ = current.emergency_stop_active;
  previous_fault_ = current.fault;
  previous_azimuth_moving_ = current.azimuth.moving;
  previous_elevation_moving_ = current.elevation.moving;
  return event;
}

std::string ProtocolEngine::host_heartbeat_timeout() {
  controller_->stop();
  controller_->set_enabled(false);
  const ControllerState& current = state();
  previous_estop_ = current.emergency_stop_active;
  previous_fault_ = current.fault;
  previous_azimuth_moving_ = current.azimuth.moving;
  previous_elevation_moving_ = current.elevation.moving;
  return "EVENT FAULT CODE=DRIVER_DISABLED DETAIL=HOST_HEARTBEAT_TIMEOUT";
}

}  // namespace radiance3d
