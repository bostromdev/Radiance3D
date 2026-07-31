#include "protocol.hpp"

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
    case FaultCode::not_homed:
      return "NOT_HOMED";
    case FaultCode::limit_reached:
      return "LIMIT_REACHED";
    case FaultCode::stopped:
      return "STOPPED";
  }
  return "UNKNOWN";
}

bool read_double(std::istringstream& input, double& value) {
  input >> value;
  return !input.fail();
}

}  // namespace

const ControllerState& ProtocolEngine::state() const { return state_; }

std::string ProtocolEngine::fault(const FaultCode code, const std::string& detail) {
  state_.fault = code;
  return "ERR " + std::string(fault_name(code)) + " " + detail;
}

std::string ProtocolEngine::status() const {
  std::ostringstream output;
  output << std::fixed << std::setprecision(3) << "OK STATUS"
         << " AZ_DEG=" << state_.azimuth.position_deg
         << " EL_DEG=" << state_.elevation.position_deg
         << " AZ_HOMED=" << (state_.azimuth.homed ? 1 : 0)
         << " EL_HOMED=" << (state_.elevation.homed ? 1 : 0)
         << " STOPPED=" << (state_.stopped ? 1 : 0)
         << " FAULT=" << fault_name(state_.fault);
  return output.str();
}

std::string ProtocolEngine::handle(const std::string& line) {
  std::istringstream input(line);
  std::string command;
  input >> command;

  if (command == "IDENTIFY") {
    return "OK IDENTIFY DEVICE=Radiance3D-SIM PROTOCOL=" +
           std::to_string(RADIANCE3D_PROTOCOL_VERSION) + " MODE=SIMULATOR";
  }
  if (command == "STATUS" || command == "POSITION") {
    return status();
  }
  if (command == "CLEAR_FAULT") {
    state_.fault = FaultCode::none;
    state_.stopped = false;
    return "OK CLEAR_FAULT";
  }
  if (command == "STOP") {
    state_.stopped = true;
    state_.fault = FaultCode::stopped;
    return "OK STOP";
  }
  if (state_.stopped) {
    return fault(FaultCode::stopped, "send CLEAR_FAULT before motion");
  }
  if (command == "HOME") {
    std::string axis;
    input >> axis;
    if (axis == "AZ" || axis == "BOTH") {
      state_.azimuth = AxisState{0.0, true, false};
    }
    if (axis == "EL" || axis == "BOTH") {
      state_.elevation = AxisState{0.0, true, false};
    }
    if (axis != "AZ" && axis != "EL" && axis != "BOTH") {
      return fault(FaultCode::invalid_argument, "HOME expects AZ, EL, or BOTH");
    }
    state_.fault = FaultCode::none;
    return "OK HOME AXIS=" + axis;
  }
  if (command == "MOVE" || command == "SCAN_STEP") {
    double azimuth = 0.0;
    double elevation = 0.0;
    double speed = 0.0;
    if (!read_double(input, azimuth) || !read_double(input, elevation) ||
        !read_double(input, speed) || speed <= 0.0) {
      return fault(FaultCode::invalid_argument, command + " expects AZ_DEG EL_DEG DEG_PER_S");
    }
    if (!state_.azimuth.homed || !state_.elevation.homed) {
      return fault(FaultCode::not_homed, "both axes must be homed");
    }
    if (azimuth < -360.0 || azimuth > 360.0 || elevation < -180.0 || elevation > 180.0) {
      return fault(FaultCode::limit_reached, "requested position exceeds protocol bounds");
    }
    state_.azimuth.position_deg = azimuth;
    state_.elevation.position_deg = elevation;
    state_.fault = FaultCode::none;
    std::ostringstream output;
    output << std::fixed << std::setprecision(3) << "OK " << command
           << " AZ_DEG=" << azimuth << " EL_DEG=" << elevation
           << " DEG_PER_S=" << speed;
    if (command == "SCAN_STEP") {
      output << " READY=1";
    }
    return output.str();
  }

  return fault(FaultCode::invalid_command, "unknown command");
}

}  // namespace radiance3d
