#pragma once

#include <cstdint>

namespace radiance3d {

// The portable axis state machine owns position, acceleration, and motion
// decisions.  A platform scheduler owns only the time-critical high/low GPIO
// edges for one STEP pin.  Keeping that boundary narrow lets host tests retain
// the cooperative fallback while physical ESP-IDF uses GPTimer.
class StepPulseScheduler {
 public:
  virtual ~StepPulseScheduler() = default;

  virtual bool initialize() = 0;
  virtual bool schedule_pulse(std::uint32_t delay_before_rising_us) = 0;
  virtual void stop() = 0;
  virtual std::uint32_t consume_completed_pulses() = 0;
  // A platform callback may be unable to arm its next edge.  The motion
  // owner consumes this sticky flag and turns it into a normal safe fault.
  virtual bool consume_scheduler_fault() { return false; }
};

}  // namespace radiance3d
