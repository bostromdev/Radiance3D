#pragma once

#include "motion_controller.hpp"

#include <cstdint>
#include <string>

namespace radiance3d {

class ProtocolEngine {
 public:
  ProtocolEngine();
  explicit ProtocolEngine(MotionController& controller,
                          std::uint32_t protocol_version = 1);

  std::string handle(const std::string& line);
  std::string service();
  std::string host_heartbeat_timeout();
  const ControllerState& state() const;

 private:
  SimulatedMotionController default_controller_;
  MotionController* controller_;
  std::uint32_t protocol_version_{1};
  std::uint32_t last_command_id_{0};
  FaultCode previous_fault_{FaultCode::none};
  bool previous_estop_{false};
  bool previous_azimuth_moving_{false};
  bool previous_elevation_moving_{false};

  std::string handle_command(const std::string& line,
                             std::uint32_t command_id);
  std::string status() const;
  std::string axis_status(AxisSelection axis) const;
  std::string diagnostics(AxisSelection axis) const;
  std::string fault(FaultCode code, const std::string& detail);
};

}  // namespace radiance3d
