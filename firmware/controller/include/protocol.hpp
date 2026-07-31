#pragma once

#include <string>

namespace radiance3d {

enum class FaultCode {
  none,
  invalid_command,
  invalid_argument,
  not_homed,
  limit_reached,
  stopped,
};

struct AxisState {
  double position_deg{0.0};
  bool homed{false};
  bool limit_active{false};
};

struct ControllerState {
  AxisState azimuth{};
  AxisState elevation{};
  FaultCode fault{FaultCode::none};
  bool stopped{false};
};

class ProtocolEngine {
 public:
  std::string handle(const std::string& line);
  const ControllerState& state() const;

 private:
  ControllerState state_{};

  std::string status() const;
  std::string fault(FaultCode code, const std::string& detail);
};

}  // namespace radiance3d
