#include "idf_step_pulse_scheduler.hpp"

#include "esp_err.h"

#include <algorithm>
#include <limits>

namespace radiance3d {
namespace {

// ESP-IDF documents GPTimer alarms below 5 us as unsuitable for reliable
// control.  TMC2209 accepts a pulse wider than its 1 us minimum, so use a
// conservative 5 us high time and minimum lead/setup interval.
constexpr std::uint64_t kPulseWidthUs = 5;
constexpr std::uint64_t kMinimumAlarmLeadUs = 5;

}  // namespace

IdfStepPulseScheduler::IdfStepPulseScheduler(const int step_pin)
    : step_pin_(step_pin) {}

IdfStepPulseScheduler::~IdfStepPulseScheduler() {
  if (timer_ != nullptr) {
    gptimer_stop(timer_);
    gptimer_disable(timer_);
    gptimer_del_timer(timer_);
  }
}

void IdfStepPulseScheduler::set_motion_task(TaskHandle_t task) {
  portENTER_CRITICAL(&lock_);
  motion_task_ = task;
  portEXIT_CRITICAL(&lock_);
}

bool IdfStepPulseScheduler::initialize() {
  if (initialized_) {
    return true;
  }
  if (step_pin_ < 0 || step_pin_ >= GPIO_NUM_MAX) {
    return false;
  }
  gpio_set_level(static_cast<gpio_num_t>(step_pin_), 0);
  gptimer_config_t config = {};
  config.clk_src = GPTIMER_CLK_SRC_DEFAULT;
  config.direction = GPTIMER_COUNT_UP;
  config.resolution_hz = 1000000;
  if (gptimer_new_timer(&config, &timer_) != ESP_OK) {
    return false;
  }
  gptimer_event_callbacks_t callbacks = {};
  callbacks.on_alarm = &IdfStepPulseScheduler::on_alarm;
  if (gptimer_register_event_callbacks(timer_, &callbacks, this) != ESP_OK ||
      gptimer_enable(timer_) != ESP_OK || gptimer_start(timer_) != ESP_OK) {
    gptimer_del_timer(timer_);
    timer_ = nullptr;
    return false;
  }
  initialized_ = true;
  return true;
}

bool IdfStepPulseScheduler::set_alarm(const std::uint64_t count) {
  // Callers hold lock_.  ESP-IDF requires an alarm configuration used from a
  // cache-safe callback to live in internal memory, not on the ISR stack.
  alarm_config_.alarm_count = count;
  alarm_config_.reload_count = 0;
  alarm_config_.flags.auto_reload_on_alarm = false;
  return gptimer_set_alarm_action(timer_, &alarm_config_) == ESP_OK;
}

void IdfStepPulseScheduler::disarm_alarm() {
  gptimer_set_alarm_action(timer_, nullptr);
}

void IdfStepPulseScheduler::mark_scheduler_fault() {
  gpio_set_level(static_cast<gpio_num_t>(step_pin_), 0);
  step_high_ = false;
  active_ = false;
  scheduler_fault_ = true;
  disarm_alarm();
}

bool IdfStepPulseScheduler::schedule_pulse(
    const std::uint32_t delay_before_rising_us) {
  if (!initialized_ || timer_ == nullptr || delay_before_rising_us == 0) {
    return false;
  }

  portENTER_CRITICAL(&lock_);
  if (active_) {
    portEXIT_CRITICAL(&lock_);
    return false;
  }

  std::uint64_t now = 0;
  const std::uint64_t delay = std::max<std::uint64_t>(
      delay_before_rising_us, kMinimumAlarmLeadUs);
  if (gptimer_get_raw_count(timer_, &now) != ESP_OK ||
      now > std::numeric_limits<std::uint64_t>::max() - delay) {
    mark_scheduler_fault();
    portEXIT_CRITICAL(&lock_);
    return false;
  }

  gpio_set_level(static_cast<gpio_num_t>(step_pin_), 0);
  step_high_ = false;
  active_ = true;
  if (!set_alarm(now + delay)) {
    mark_scheduler_fault();
    portEXIT_CRITICAL(&lock_);
    return false;
  }
  portEXIT_CRITICAL(&lock_);
  return true;
}

void IdfStepPulseScheduler::stop() {
  if (!initialized_ || timer_ == nullptr) {
    return;
  }
  portENTER_CRITICAL(&lock_);
  active_ = false;
  step_high_ = false;
  gpio_set_level(static_cast<gpio_num_t>(step_pin_), 0);
  disarm_alarm();
  portEXIT_CRITICAL(&lock_);
}

std::uint32_t IdfStepPulseScheduler::consume_completed_pulses() {
  portENTER_CRITICAL(&lock_);
  const std::uint32_t completed = completed_pulses_;
  completed_pulses_ = 0;
  portEXIT_CRITICAL(&lock_);
  return completed;
}

bool IdfStepPulseScheduler::consume_scheduler_fault() {
  portENTER_CRITICAL(&lock_);
  const bool fault = scheduler_fault_;
  scheduler_fault_ = false;
  portEXIT_CRITICAL(&lock_);
  return fault;
}

void IdfStepPulseScheduler::emergency_stop_from_isr() {
  if (!initialized_ || timer_ == nullptr) {
    return;
  }
  portENTER_CRITICAL_ISR(&lock_);
  active_ = false;
  step_high_ = false;
  gpio_set_level(static_cast<gpio_num_t>(step_pin_), 0);
  disarm_alarm();
  portEXIT_CRITICAL_ISR(&lock_);
}

bool IdfStepPulseScheduler::on_alarm(
    gptimer_handle_t, const gptimer_alarm_event_data_t* const event_data,
    void* const user_context) {
  return static_cast<IdfStepPulseScheduler*>(user_context)->handle_alarm(
      event_data);
}

bool IdfStepPulseScheduler::handle_alarm(
    const gptimer_alarm_event_data_t* const event_data) {
  BaseType_t higher_priority_woken = pdFALSE;
  TaskHandle_t task_to_wake = nullptr;

  portENTER_CRITICAL_ISR(&lock_);
  if (!active_) {
    portEXIT_CRITICAL_ISR(&lock_);
    return false;
  }

  if (!step_high_) {
    gpio_set_level(static_cast<gpio_num_t>(step_pin_), 1);
    step_high_ = true;
    std::uint64_t now = event_data->count_value;
    if (gptimer_get_raw_count(timer_, &now) != ESP_OK ||
        now > std::numeric_limits<std::uint64_t>::max() -
                  kMinimumAlarmLeadUs ||
        !set_alarm(std::max(event_data->count_value + kPulseWidthUs,
                            now + kMinimumAlarmLeadUs))) {
      mark_scheduler_fault();
      task_to_wake = motion_task_;
    }
  } else {
    gpio_set_level(static_cast<gpio_num_t>(step_pin_), 0);
    step_high_ = false;
    active_ = false;
    disarm_alarm();
    ++completed_pulses_;
    task_to_wake = motion_task_;
  }
  portEXIT_CRITICAL_ISR(&lock_);

  if (task_to_wake != nullptr) {
    vTaskNotifyGiveFromISR(task_to_wake, &higher_priority_woken);
  }
  return higher_priority_woken == pdTRUE;
}

}  // namespace radiance3d
