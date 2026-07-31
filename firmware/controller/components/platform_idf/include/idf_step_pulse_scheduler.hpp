#pragma once

#include "step_pulse_scheduler.hpp"

#include "driver/gptimer.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cstdint>

namespace radiance3d {

// GPTimer emits a single STEP high/low pulse.  AxisController remains the sole
// owner of position and acceleration state and consumes completed pulses in the
// motion task.  The ISR never performs UART, allocation, logging, or motion
// planning.
class IdfStepPulseScheduler final : public StepPulseScheduler {
 public:
  explicit IdfStepPulseScheduler(int step_pin);
  ~IdfStepPulseScheduler() override;

  IdfStepPulseScheduler(const IdfStepPulseScheduler&) = delete;
  IdfStepPulseScheduler& operator=(const IdfStepPulseScheduler&) = delete;

  void set_motion_task(TaskHandle_t task);
  bool initialize() override;
  bool schedule_pulse(std::uint32_t delay_before_rising_us) override;
  void stop() override;
  std::uint32_t consume_completed_pulses() override;
  bool consume_scheduler_fault() override;

  // Called only by the emergency-input ISR. It pulls STEP low and disarms the
  // scheduler; the safety/motion task performs driver disable and fault logic.
  void IRAM_ATTR emergency_stop_from_isr();

 private:
  static bool IRAM_ATTR on_alarm(gptimer_handle_t timer,
                                 const gptimer_alarm_event_data_t* event_data,
                                 void* user_context);
  bool IRAM_ATTR handle_alarm(const gptimer_alarm_event_data_t* event_data);
  bool IRAM_ATTR set_alarm(std::uint64_t count);
  void IRAM_ATTR disarm_alarm();
  void IRAM_ATTR mark_scheduler_fault();

  int step_pin_;
  gptimer_handle_t timer_{nullptr};
  TaskHandle_t motion_task_{nullptr};
  portMUX_TYPE lock_ = portMUX_INITIALIZER_UNLOCKED;
  // The runtime owns this scheduler as static storage, so this member resides
  // in DRAM when the cache-safe callback passes it to GPTimer.
  gptimer_alarm_config_t alarm_config_{};
  bool initialized_{false};
  bool active_{false};
  bool step_high_{false};
  bool scheduler_fault_{false};
  std::uint32_t completed_pulses_{0};
};

}  // namespace radiance3d
