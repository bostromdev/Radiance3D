#include "protocol.hpp"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

#ifndef RADIANCE3D_PROTOCOL_VERSION
#define RADIANCE3D_PROTOCOL_VERSION 1
#endif

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
    case FaultCode::stopped:
      return "STOPPED";
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

}  // namespace

ProtocolEngine::ProtocolEngine()
    : default_controller_(provisional_simulator_config()), controller_(&default_controller_) {}

ProtocolEngine::ProtocolEngine(MotionController& controller)
    : default_controller_(provisional_simulator_config()), controller_(&controller) {}

const ControllerState& ProtocolEngine::state() const { return controller_->state(); }

std::string ProtocolEngine::fault(const FaultCode code, const std::string& detail) {
  controller_->report_fault(code);
  return "ERR " + std::string(fault_name(code)) + " " + detail;
}

std::string ProtocolEngine::status() const {
  const ControllerState& controller_state = state();
  std::ostringstream output;
  output << std::fixed << std::setprecision(3) << "OK STATUS"
         << " AZ_DEG=" << controller_state.azimuth.commanded_position_deg
         << " EL_DEG=" << controller_state.elevation.commanded_position_deg
         << " POSITION_KIND=COMMANDED"
         << " AZ_HOMED=" << (controller_state.azimuth.homed ? 1 : 0)
         << " EL_HOMED=" << (controller_state.elevation.homed ? 1 : 0)
         << " AZ_TRUSTED=" << (controller_state.azimuth.position_trusted ? 1 : 0)
         << " EL_TRUSTED=" << (controller_state.elevation.position_trusted ? 1 : 0)
         << " DRIVERS_ENABLED="
         << ((controller_state.azimuth.enabled && controller_state.elevation.enabled) ? 1 : 0)
         << " STOPPED=" << (controller_state.stopped ? 1 : 0)
         << " ESTOP=" << (controller_state.emergency_stop_active ? 1 : 0)
         << " FAULT=" << fault_name(controller_state.fault);
  return output.str();
}

std::string ProtocolEngine::handle(const std::string& line) {
  std::istringstream input(line);
  std::string command;
  input >> command;

  if (command == "IDENTIFY") {
    if (!no_extra_arguments(input)) {
      return fault(FaultCode::invalid_argument, "IDENTIFY expects no arguments");
    }
    return "OK IDENTIFY DEVICE=Radiance3D-SIM PROTOCOL=" +
           std::to_string(RADIANCE3D_PROTOCOL_VERSION) + " MODE=SIMULATOR";
  }
  if (command == "STATUS" || command == "POSITION") {
    if (!no_extra_arguments(input)) {
      return fault(FaultCode::invalid_argument, command + " expects no arguments");
    }
    return status();
  }
  if (command == "CLEAR_FAULT") {
    if (!no_extra_arguments(input)) {
      return fault(FaultCode::invalid_argument, "CLEAR_FAULT expects no arguments");
    }
    const MotionResult result = controller_->clear_fault();
    if (!result.ok) {
      return fault(result.fault, "emergency-stop input must be released first");
    }
    return "OK CLEAR_FAULT";
  }
  if (command == "STOP") {
    if (!no_extra_arguments(input)) {
      return fault(FaultCode::invalid_argument, "STOP expects no arguments");
    }
    controller_->stop();
    return "OK STOP";
  }
  if (command == "E_STOP") {
    if (!no_extra_arguments(input)) {
      return fault(FaultCode::invalid_argument, "E_STOP expects no arguments");
    }
    controller_->emergency_stop();
    return "OK E_STOP";
  }
  if (command == "ENABLE") {
    int enabled = -1;
    input >> enabled;
    if (input.fail() || (enabled != 0 && enabled != 1) || !no_extra_arguments(input)) {
      return fault(FaultCode::invalid_argument, "ENABLE expects 0 or 1");
    }
    const MotionResult result = controller_->set_enabled(enabled == 1);
    if (!result.ok && enabled == 1) {
      return fault(result.fault, "driver state change failed");
    }
    return "OK ENABLE VALUE=" + std::to_string(enabled);
  }
  if (command == "HOME") {
    std::string axis;
    input >> axis;
    if (!no_extra_arguments(input) || (axis != "AZ" && axis != "EL" && axis != "BOTH")) {
      return fault(FaultCode::invalid_argument, "HOME expects AZ, EL, or BOTH");
    }
    const AxisSelection selection =
        axis == "AZ"     ? AxisSelection::azimuth
        : axis == "EL"  ? AxisSelection::elevation
                        : AxisSelection::both;
    const MotionResult result = controller_->home(selection);
    if (!result.ok) {
      return fault(result.fault, "homing rejected by motion controller");
    }
    return "OK HOME AXIS=" + axis;
  }
  if (command == "MOVE" || command == "SCAN_STEP") {
    double azimuth = 0.0;
    double elevation = 0.0;
    double speed = 0.0;
    if (!read_double(input, azimuth) || !read_double(input, elevation) ||
        !read_double(input, speed) || speed <= 0.0 || !no_extra_arguments(input)) {
      return fault(FaultCode::invalid_argument, command + " expects AZ_DEG EL_DEG DEG_PER_S");
    }
    const MotionResult result = controller_->move_absolute(azimuth, elevation, speed);
    if (!result.ok) {
      return fault(result.fault, "motion rejected by configured controller limits or state");
    }
    std::ostringstream output;
    output << std::fixed << std::setprecision(3) << "OK " << command
           << " AZ_DEG=" << azimuth << " EL_DEG=" << elevation
           << " DEG_PER_S=" << speed;
    if (command == "SCAN_STEP") {
      output << " READY=1 POSITION_KIND=COMMANDED";
    }
    return output.str();
  }

  return fault(FaultCode::invalid_command, "unknown command");
}

}  // namespace radiance3d
