#include "controller_runtime.hpp"

#include "axis_controller.hpp"
#include "hardware_config.hpp"
#include "hardware_profile_generated.hpp"
#include "idf_hardware_platform.hpp"
#include "idf_step_pulse_scheduler.hpp"
#include "physical_motion_controller.hpp"
#include "protocol.hpp"
#include "tmc2209_driver.hpp"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_err.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include <cstdio>
#include <cstring>
#include <string>

namespace radiance3d {
namespace {

constexpr char kAppTag[] = "APP";
constexpr char kProtocolTag[] = "PROTOCOL";
constexpr char kMotionTag[] = "MOTION";
constexpr char kSafetyTag[] = "SAFETY";
constexpr char kDiagnosticsTag[] = "DIAGNOSTICS";
constexpr std::size_t kMaximumHostLine = 255;
// STATUS includes two full axis payloads.  Leave room for additive protocol
// fields instead of silently truncating a valid v1 response.
constexpr std::size_t kMaximumOutboundLine = 1024;
constexpr std::uint32_t kMotionTaskPriority = 8;
constexpr std::uint32_t kSafetyTaskPriority = 9;
constexpr std::uint32_t kProtocolTaskPriority = 5;
constexpr std::uint32_t kDiagnosticsTaskPriority = 3;

constexpr EventBits_t kAzimuthEnabled = BIT0;
constexpr EventBits_t kElevationEnabled = BIT1;
constexpr EventBits_t kMotionActive = BIT2;

enum class MotionMessageType : std::uint8_t {
  host_command,
  emergency_stop,
  heartbeat_timeout,
  diagnostics_tick,
};

struct MotionMessage {
  MotionMessageType type{MotionMessageType::host_command};
  std::uint32_t token{0};
  char line[kMaximumHostLine + 1]{};
};

struct OutboundMessage {
  std::uint32_t token{0};
  char line[kMaximumOutboundLine]{};
};

struct SafetyMessage {
  bool host_activity{false};
};

struct IsrInput {
  bool emergency_stop{false};
  int pin{-1};
  bool active_low{true};
  int azimuth_enable_pin{-1};
  bool azimuth_disable_level{true};
  int elevation_enable_pin{-1};
  bool elevation_disable_level{true};
};

struct Runtime {
  Runtime()
      : config(provisional_esp32_dev_config()),
        azimuth_driver(platform, config.azimuth.driver),
        elevation_driver(platform, config.elevation.driver),
        azimuth_timer(config.azimuth.driver.step_pin),
        elevation_timer(config.elevation.driver.step_pin),
        azimuth_axis(platform, azimuth_driver, config.azimuth.axis,
                     &azimuth_timer),
        elevation_axis(platform, elevation_driver, config.elevation.axis,
                       &elevation_timer),
        controller(platform, azimuth_axis, elevation_axis, config),
        protocol(controller, config.protocol_version) {
    emergency_input.emergency_stop = true;
    emergency_input.pin = config.emergency_stop_pin;
    emergency_input.active_low = config.emergency_stop_active_low;
    emergency_input.azimuth_enable_pin = config.azimuth.driver.enable_pin;
    emergency_input.azimuth_disable_level =
        config.azimuth.driver.enable_active_low;
    emergency_input.elevation_enable_pin = config.elevation.driver.enable_pin;
    emergency_input.elevation_disable_level =
        config.elevation.driver.enable_active_low;
    azimuth_home_input.emergency_stop = false;
    azimuth_home_input.pin = config.azimuth.axis.home_switch_pin;
    elevation_home_input.emergency_stop = false;
    elevation_home_input.pin = config.elevation.axis.home_switch_pin;
  }

  IdfHardwarePlatform platform;
  PhysicalControllerConfig config;
  Tmc2209Driver azimuth_driver;
  Tmc2209Driver elevation_driver;
  IdfStepPulseScheduler azimuth_timer;
  IdfStepPulseScheduler elevation_timer;
  AxisController azimuth_axis;
  AxisController elevation_axis;
  PhysicalMotionController controller;
  ProtocolEngine protocol;
  QueueHandle_t motion_queue{nullptr};
  QueueHandle_t outbound_queue{nullptr};
  QueueHandle_t safety_queue{nullptr};
  EventGroupHandle_t state_events{nullptr};
  TaskHandle_t motion_task{nullptr};
  TaskHandle_t safety_task{nullptr};
  portMUX_TYPE emergency_lock = portMUX_INITIALIZER_UNLOCKED;
  bool emergency_stop_pending{false};
  bool controller_initialized{false};
  bool host_uart_initialized{false};
  const char* reset_reason{"UNKNOWN"};
  IsrInput emergency_input{};
  IsrInput azimuth_home_input{};
  IsrInput elevation_home_input{};
};

// Never call the C++ static-local accessor from an IRAM GPIO ISR: its guard
// path lives in flash.  The Runtime instance itself has static DRAM storage.
DRAM_ATTR Runtime* g_runtime_for_isr{nullptr};

Runtime& runtime() {
  static Runtime instance;
  return instance;
}

bool copy_line(char* const destination, const std::size_t destination_size,
               const std::string& source) {
  if (destination_size == 0 || source.size() >= destination_size) {
    return false;
  }
  const std::size_t length = source.size();
  std::memcpy(destination, source.data(), length);
  destination[length] = '\0';
  return true;
}

bool queue_motion_message(const MotionMessage& message) {
  Runtime& state = runtime();
  if (state.motion_queue == nullptr ||
      xQueueSend(state.motion_queue, &message, 0) != pdPASS) {
    return false;
  }
  if (state.motion_task != nullptr) {
    xTaskNotifyGive(state.motion_task);
  }
  return true;
}

void request_emergency_stop() {
  Runtime& state = runtime();
  portENTER_CRITICAL(&state.emergency_lock);
  state.emergency_stop_pending = true;
  portEXIT_CRITICAL(&state.emergency_lock);
  if (state.motion_task != nullptr) {
    xTaskNotifyGive(state.motion_task);
  }
}

bool take_emergency_stop_request() {
  Runtime& state = runtime();
  portENTER_CRITICAL(&state.emergency_lock);
  const bool pending = state.emergency_stop_pending;
  state.emergency_stop_pending = false;
  portEXIT_CRITICAL(&state.emergency_lock);
  return pending;
}

bool queue_outbound(const std::string& line, const std::uint32_t token = 0) {
  Runtime& state = runtime();
  if (state.outbound_queue == nullptr) {
    return false;
  }
  OutboundMessage message;
  message.token = token;
  if (!copy_line(message.line, sizeof(message.line), line)) {
    ESP_LOGE(kProtocolTag, "outbound protocol line exceeds %u bytes",
             static_cast<unsigned>(kMaximumOutboundLine - 1));
    if (token == 0 ||
        !copy_line(message.line, sizeof(message.line),
                   "ERR INTERNAL response exceeds maximum line length")) {
      return false;
    }
  }
  if (xQueueSend(state.outbound_queue, &message, 0) != pdPASS) {
    ESP_LOGW(kProtocolTag, "outbound queue full; dropping line");
    return false;
  }
  return true;
}

void update_shared_state() {
  Runtime& state = runtime();
  EventBits_t clear_bits = kAzimuthEnabled | kElevationEnabled | kMotionActive;
  EventBits_t set_bits = 0;
  const ControllerState& controller_state = state.controller.state();
  if (controller_state.azimuth.enabled) {
    set_bits |= kAzimuthEnabled;
  }
  if (controller_state.elevation.enabled) {
    set_bits |= kElevationEnabled;
  }
  if (controller_state.azimuth.moving || controller_state.elevation.moving) {
    set_bits |= kMotionActive;
  }
  xEventGroupClearBits(state.state_events, clear_bits);
  if (set_bits != 0) {
    xEventGroupSetBits(state.state_events, set_bits);
  }
}

void safe_outputs() {
  Runtime& state = runtime();
  const PhysicalAxisDefinition axes[] = {state.config.azimuth, state.config.elevation};
  for (const PhysicalAxisDefinition& axis : axes) {
    state.platform.configure_pin(axis.driver.step_pin, PinMode::output);
    state.platform.write_pin(axis.driver.step_pin, false);
    state.platform.configure_pin(axis.driver.direction_pin, PinMode::output);
    state.platform.write_pin(axis.driver.direction_pin, false);
    state.platform.configure_pin(axis.driver.enable_pin, PinMode::output);
    state.platform.write_pin(axis.driver.enable_pin, axis.driver.enable_active_low);
  }
}

bool initialize_host_uart() {
  uart_config_t config = {};
  config.baud_rate = static_cast<int>(generated_profile::kHostUartBaud);
  config.data_bits = UART_DATA_8_BITS;
  config.parity = UART_PARITY_DISABLE;
  config.stop_bits = UART_STOP_BITS_1;
  config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
  config.source_clk = UART_SCLK_DEFAULT;
  if (uart_param_config(UART_NUM_0, &config) != ESP_OK ||
      uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
                   UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK) {
    return false;
  }
  const esp_err_t install =
      uart_driver_install(UART_NUM_0, 1024, 1024, 0, nullptr, 0);
  return install == ESP_OK || install == ESP_ERR_INVALID_STATE;
}

void write_host_line(const char* const line) {
  const std::size_t length = std::strlen(line);
  uart_write_bytes(UART_NUM_0, line, length);
  uart_write_bytes(UART_NUM_0, "\n", 1);
  uart_wait_tx_done(UART_NUM_0, pdMS_TO_TICKS(50));
}

void service_motion_message(const MotionMessage& message) {
  Runtime& state = runtime();
  switch (message.type) {
    case MotionMessageType::host_command:
      queue_outbound(state.protocol.handle(message.line), message.token);
      break;
    case MotionMessageType::emergency_stop:
      state.controller.emergency_stop();
      break;
    case MotionMessageType::heartbeat_timeout:
      queue_outbound(state.protocol.host_heartbeat_timeout());
      break;
    case MotionMessageType::diagnostics_tick:
      // This still runs through the motion owner, but only reads UART while
      // both axes are idle. Active movement never waits on a TMC timeout.
      state.controller.service_diagnostics();
      break;
  }
}

void register_task_watchdog() {
  if (esp_task_wdt_add(nullptr) != ESP_OK) {
    ESP_LOGW(kAppTag, "task watchdog registration unavailable for this task");
  }
}

void motion_task(void*) {
  Runtime& state = runtime();
  register_task_watchdog();
  state.azimuth_timer.set_motion_task(xTaskGetCurrentTaskHandle());
  state.elevation_timer.set_motion_task(xTaskGetCurrentTaskHandle());
  queue_outbound(std::string("EVENT STARTUP READY=") +
                 (state.controller_initialized ? "1" : "0") +
                 " DRIVERS_ENABLED=0 BOARD=" + state.config.board_name +
                 " RESET=" + state.reset_reason);
  for (;;) {
    if (take_emergency_stop_request()) {
      // Any active e-stop edge is deliberately fail-safe-latched. The ISR has
      // already dropped STEP and the enable pins; this establishes coherent
      // controller state even if the input bounces before debounce completes.
      state.controller.emergency_stop();
    }
    MotionMessage message;
    while (xQueueReceive(state.motion_queue, &message, 0) == pdPASS) {
      service_motion_message(message);
    }
    const std::string event = state.protocol.service();
    if (!event.empty()) {
      queue_outbound(event);
    }
    update_shared_state();
    esp_task_wdt_reset();
    // GPIO/STEP callbacks and all queue producers notify this task. A timeout
    // keeps debounce, timeouts, and diagnostics progressing even when idle.
    ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1));
  }
}

bool emergency_active() {
  Runtime& state = runtime();
  const bool level = state.platform.read_pin(state.config.emergency_stop_pin);
  return state.config.emergency_stop_active_low ? !level : level;
}

void safety_task(void*) {
  Runtime& state = runtime();
  register_task_watchdog();
  bool host_seen = false;
  bool heartbeat_tripped = false;
  std::int64_t last_host_activity_us = 0;
  for (;;) {
    SafetyMessage incoming;
    while (xQueueReceive(state.safety_queue, &incoming, 0) == pdPASS) {
      if (incoming.host_activity) {
        host_seen = true;
        heartbeat_tripped = false;
        last_host_activity_us = esp_timer_get_time();
      }
    }

    // GPIO ISR notification and host activity both use this wakeup.  Do not
    // delay here: controller-side input debounce runs outside the ISR, while
    // an asserted edge is immediately made fail-safe by request_emergency_stop.
    ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(25));
    if (emergency_active()) {
      request_emergency_stop();
    }

    const EventBits_t flags = xEventGroupGetBits(state.state_events);
    const bool drivers_enabled =
        (flags & (kAzimuthEnabled | kElevationEnabled)) != 0;
    const std::int64_t now_us = esp_timer_get_time();
    if (host_seen && !heartbeat_tripped && drivers_enabled &&
        now_us - last_host_activity_us >=
            static_cast<std::int64_t>(CONFIG_RADIANCE3D_HOST_HEARTBEAT_TIMEOUT_MS) *
                1000) {
      MotionMessage timeout;
      timeout.type = MotionMessageType::heartbeat_timeout;
      if (queue_motion_message(timeout)) {
        heartbeat_tripped = true;
      }
    }
    esp_task_wdt_reset();
  }
}

void diagnostics_task(void*) {
  register_task_watchdog();
  TickType_t next_wake = xTaskGetTickCount();
  for (;;) {
    vTaskDelayUntil(&next_wake,
                    pdMS_TO_TICKS(CONFIG_RADIANCE3D_DIAGNOSTICS_INTERVAL_MS));
    MotionMessage tick;
    tick.type = MotionMessageType::diagnostics_tick;
    if (!queue_motion_message(tick)) {
      ESP_LOGW(kDiagnosticsTag, "motion queue full while scheduling diagnostics");
    }
    esp_task_wdt_reset();
  }
}

void protocol_task(void*) {
  Runtime& state = runtime();
  register_task_watchdog();
  char line[kMaximumHostLine + 1]{};
  std::size_t length = 0;
  std::uint32_t token = 0;
  bool discarding_overlong_line = false;
  for (;;) {
    OutboundMessage pending;
    while (xQueueReceive(state.outbound_queue, &pending, 0) == pdPASS) {
      write_host_line(pending.line);
    }
    std::uint8_t character = 0;
    const int received = uart_read_bytes(UART_NUM_0, &character, 1,
                                         pdMS_TO_TICKS(20));
    if (received <= 0) {
      esp_task_wdt_reset();
      continue;
    }
    if (character == '\r') {
      continue;
    }
    if (discarding_overlong_line) {
      if (character == '\n') {
        discarding_overlong_line = false;
      }
      continue;
    }
    if (character != '\n') {
      if (character >= 0x20 && character <= 0x7e &&
          length < kMaximumHostLine) {
        line[length++] = static_cast<char>(character);
      } else {
        length = 0;
        discarding_overlong_line = true;
        write_host_line(
            "ERR INVALID_ARGUMENT input line must be printable ASCII and at most 255 bytes");
      }
      continue;
    }
    line[length] = '\0';
    ++token;
    SafetyMessage activity;
    activity.host_activity = true;
    if (xQueueSend(state.safety_queue, &activity, 0) == pdPASS &&
        state.safety_task != nullptr) {
      xTaskNotifyGive(state.safety_task);
    }
    MotionMessage command;
    command.type = MotionMessageType::host_command;
    command.token = token;
    std::memcpy(command.line, line, length + 1);
    length = 0;
    if (!queue_motion_message(command)) {
      write_host_line("ERR BUSY command queue full");
      continue;
    }
    // Commands are parsed synchronously by the motion owner, while queued
    // asynchronous events continue to be forwarded before the response.
    bool response_sent = false;
    while (!response_sent) {
      OutboundMessage outbound;
      if (xQueueReceive(state.outbound_queue, &outbound, pdMS_TO_TICKS(250)) !=
          pdPASS) {
        write_host_line("ERR INTERNAL command response timeout");
        break;
      }
      write_host_line(outbound.line);
      response_sent = outbound.token == token;
    }
    esp_task_wdt_reset();
  }
}

void IRAM_ATTR input_isr(void* const argument) {
  Runtime* const state = g_runtime_for_isr;
  if (state == nullptr) {
    return;
  }
  const IsrInput* const input = static_cast<const IsrInput*>(argument);
  BaseType_t higher_priority_woken = pdFALSE;
  if (input->emergency_stop) {
    const bool level =
        gpio_get_level(static_cast<gpio_num_t>(input->pin)) != 0;
    const bool active = input->active_low ? !level : level;
    if (active) {
      state->azimuth_timer.emergency_stop_from_isr();
      state->elevation_timer.emergency_stop_from_isr();
      // Driver disable is intentionally a direct cache-safe GPIO write here,
      // not a queue operation that could be full. The motion task latches
      // fault/trust state immediately after being notified.
      gpio_set_level(static_cast<gpio_num_t>(input->azimuth_enable_pin),
                     input->azimuth_disable_level ? 1 : 0);
      gpio_set_level(static_cast<gpio_num_t>(input->elevation_enable_pin),
                     input->elevation_disable_level ? 1 : 0);
      portENTER_CRITICAL_ISR(&state->emergency_lock);
      state->emergency_stop_pending = true;
      portEXIT_CRITICAL_ISR(&state->emergency_lock);
    }
    if (state->motion_task != nullptr && active) {
      vTaskNotifyGiveFromISR(state->motion_task, &higher_priority_woken);
    }
    if (state->safety_task != nullptr) {
      vTaskNotifyGiveFromISR(state->safety_task, &higher_priority_woken);
    }
  } else if (state->motion_task != nullptr) {
    vTaskNotifyGiveFromISR(state->motion_task, &higher_priority_woken);
  }
  if (higher_priority_woken == pdTRUE) {
    portYIELD_FROM_ISR();
  }
}

bool configure_input_interrupts() {
  Runtime& state = runtime();
  const esp_err_t install = gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
  if (install != ESP_OK && install != ESP_ERR_INVALID_STATE) {
    return false;
  }
  const int pins[] = {state.config.emergency_stop_pin,
                      state.config.azimuth.axis.home_switch_pin,
                      state.config.elevation.axis.home_switch_pin};
  IsrInput* const inputs[] = {&state.emergency_input, &state.azimuth_home_input,
                               &state.elevation_home_input};
  for (std::size_t index = 0; index < 3; ++index) {
    if (pins[index] < 0) {
      continue;
    }
    const gpio_num_t pin = static_cast<gpio_num_t>(pins[index]);
    if (gpio_set_intr_type(pin, GPIO_INTR_ANYEDGE) != ESP_OK ||
        gpio_isr_handler_add(pin, input_isr, inputs[index]) != ESP_OK) {
      return false;
    }
  }
  return true;
}

}  // namespace

bool controller_runtime_initialize(const char* const reset_reason) {
  Runtime& state = runtime();
  g_runtime_for_isr = &state;
  state.reset_reason = reset_reason == nullptr ? "UNKNOWN" : reset_reason;
  safe_outputs();
  state.motion_queue = xQueueCreate(16, sizeof(MotionMessage));
  state.outbound_queue = xQueueCreate(24, sizeof(OutboundMessage));
  state.safety_queue = xQueueCreate(8, sizeof(SafetyMessage));
  state.state_events = xEventGroupCreate();
  state.host_uart_initialized = initialize_host_uart();
  if (state.motion_queue == nullptr || state.outbound_queue == nullptr ||
      state.safety_queue == nullptr || state.state_events == nullptr ||
      !state.host_uart_initialized) {
    ESP_LOGE(kAppTag, "could not initialize native queues, event group, or UART0");
    return false;
  }
  const GpioValidationResult gpio = validate_esp32_gpio(state.config);
  if (gpio.bootstrapping_pin_mask != 0) {
    const std::string warning =
        "EVENT WARNING CODE=ESP32_BOOTSTRAP_GPIO MASK=" +
        std::to_string(gpio.bootstrapping_pin_mask);
    queue_outbound(warning);
    ESP_LOGW("CONFIG", "ESP32 bootstrap GPIO mask=%llu",
             static_cast<unsigned long long>(gpio.bootstrapping_pin_mask));
  }
  state.controller_initialized = state.controller.initialize();
  if (!configure_input_interrupts()) {
    ESP_LOGE(kSafetyTag, "could not configure input interrupts");
    state.controller_initialized = false;
  }
  update_shared_state();
  return state.controller_initialized;
}

void controller_runtime_start() {
  Runtime& state = runtime();
  esp_task_wdt_config_t watchdog = {};
  watchdog.timeout_ms = CONFIG_RADIANCE3D_TASK_WDT_TIMEOUT_MS;
  watchdog.idle_core_mask = 0;
  watchdog.trigger_panic = true;
  const esp_err_t wdt = esp_task_wdt_init(&watchdog);
  if (wdt != ESP_OK && wdt != ESP_ERR_INVALID_STATE) {
    ESP_LOGW(kAppTag, "could not initialize task watchdog: %s", esp_err_to_name(wdt));
  }
  xTaskCreate(motion_task, "radiance_motion", CONFIG_RADIANCE3D_MOTION_TASK_STACK,
              nullptr, kMotionTaskPriority, &state.motion_task);
  xTaskCreate(safety_task, "radiance_safety", CONFIG_RADIANCE3D_SAFETY_TASK_STACK,
              nullptr, kSafetyTaskPriority, &state.safety_task);
  xTaskCreate(protocol_task, "radiance_protocol",
              CONFIG_RADIANCE3D_PROTOCOL_TASK_STACK, nullptr,
              kProtocolTaskPriority, nullptr);
  xTaskCreate(diagnostics_task, "radiance_diagnostics",
              CONFIG_RADIANCE3D_DIAGNOSTICS_TASK_STACK, nullptr,
              kDiagnosticsTaskPriority, nullptr);
  ESP_LOGI(kAppTag, "tasks started without core affinity");
}

}  // namespace radiance3d
