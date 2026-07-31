#pragma once

#include "motion_controller.hpp"

#include <string>

namespace radiance3d {

class ProtocolEngine {
 public:
  ProtocolEngine();
  explicit ProtocolEngine(MotionController& controller);

  std::string handle(const std::string& line);
  const ControllerState& state() const;

 private:
  SimulatedMotionController default_controller_;
  MotionController* controller_;

  std::string status() const;
  std::string fault(FaultCode code, const std::string& detail);
};

}  // namespace radiance3d
