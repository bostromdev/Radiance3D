#include "idf_hardware_platform.hpp"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"

#include <limits>

namespace radiance3d {

int IdfHardwarePlatform::uart_port(const std::uint8_t channel) {
  if (channel == 1) {
    return UART_NUM_1;
  }
  if (channel == 2) {
    return UART_NUM_2;
  }
  return -1;
}

bool IdfHardwarePlatform::configure_pin(const int pin, const PinMode mode) {
  if (pin < 0 || pin >= GPIO_NUM_MAX) {
    return false;
  }
  gpio_config_t config = {};
  config.pin_bit_mask = 1ULL << static_cast<unsigned>(pin);
  config.intr_type = GPIO_INTR_DISABLE;
  if (mode == PinMode::output) {
    config.mode = GPIO_MODE_OUTPUT;
    config.pull_up_en = GPIO_PULLUP_DISABLE;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
  } else {
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = mode == PinMode::input_pullup ? GPIO_PULLUP_ENABLE
                                                       : GPIO_PULLUP_DISABLE;
    config.pull_down_en = mode == PinMode::input_pulldown
                              ? GPIO_PULLDOWN_ENABLE
                              : GPIO_PULLDOWN_DISABLE;
  }
  return gpio_config(&config) == ESP_OK;
}

void IdfHardwarePlatform::write_pin(const int pin, const bool high) {
  if (pin >= 0 && pin < GPIO_NUM_MAX) {
    gpio_set_level(static_cast<gpio_num_t>(pin), high ? 1 : 0);
  }
}

bool IdfHardwarePlatform::read_pin(const int pin) const {
  return pin >= 0 && pin < GPIO_NUM_MAX &&
         gpio_get_level(static_cast<gpio_num_t>(pin)) != 0;
}

std::uint64_t IdfHardwarePlatform::monotonic_micros() const {
  return static_cast<std::uint64_t>(esp_timer_get_time());
}

bool IdfHardwarePlatform::begin_uart(const std::uint8_t channel,
                                     const int tx_pin, const int rx_pin,
                                     const std::uint32_t baud) {
  const int port = uart_port(channel);
  if (port < 0 || tx_pin < 0 || rx_pin < 0 || baud == 0) {
    return false;
  }
  uart_config_t config = {};
  config.baud_rate = static_cast<int>(baud);
  config.data_bits = UART_DATA_8_BITS;
  config.parity = UART_PARITY_DISABLE;
  config.stop_bits = UART_STOP_BITS_1;
  config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
  config.source_clk = UART_SCLK_DEFAULT;
  if (uart_param_config(static_cast<uart_port_t>(port), &config) != ESP_OK ||
      uart_set_pin(static_cast<uart_port_t>(port), tx_pin, rx_pin,
                   UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK) {
    return false;
  }
  if (!uart_installed_[channel]) {
    if (uart_driver_install(static_cast<uart_port_t>(port), 256, 256, 0,
                            nullptr, 0) != ESP_OK) {
      return false;
    }
    uart_installed_[channel] = true;
  }
  return true;
}

bool IdfHardwarePlatform::configure_uart_half_duplex(
    const std::uint8_t channel, const bool enabled) {
  const int port = uart_port(channel);
  if (port < 0 || !uart_installed_[channel]) {
    return false;
  }
  // A TMC2209 PDN_UART bus is electrically single-wire, but it is not an
  // RS-485 bus: RS485 half-duplex mode drives RTS and does not join TX/RX.
  // The carrier wiring joins the ESP TX (through its required resistor) and
  // RX at PDN_UART.  Keep the peripheral in normal UART mode and let the
  // portable driver filter the expected write echo.
  static_cast<void>(enabled);
  return uart_set_mode(static_cast<uart_port_t>(port), UART_MODE_UART) == ESP_OK;
}

void IdfHardwarePlatform::flush_uart_input(const std::uint8_t channel) {
  const int port = uart_port(channel);
  if (port >= 0 && uart_installed_[channel]) {
    uart_flush_input(static_cast<uart_port_t>(port));
  }
}

bool IdfHardwarePlatform::write_uart(const std::uint8_t channel,
                                     const std::uint8_t* const data,
                                     const std::size_t length) {
  const int port = uart_port(channel);
  if (port < 0 || !uart_installed_[channel] || data == nullptr || length == 0 ||
      length > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
    return false;
  }
  const int written = uart_write_bytes(static_cast<uart_port_t>(port), data,
                                       static_cast<std::size_t>(length));
  if (written != static_cast<int>(length)) {
    return false;
  }
  return uart_wait_tx_done(static_cast<uart_port_t>(port), pdMS_TO_TICKS(20)) ==
         ESP_OK;
}

std::size_t IdfHardwarePlatform::read_uart(const std::uint8_t channel,
                                           std::uint8_t* const data,
                                           const std::size_t maximum_length,
                                           const std::uint32_t timeout_ms) {
  const int port = uart_port(channel);
  if (port < 0 || !uart_installed_[channel] || data == nullptr ||
      maximum_length == 0 ||
      maximum_length > static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max())) {
    return 0;
  }
  const int received = uart_read_bytes(
      static_cast<uart_port_t>(port), data, maximum_length,
      pdMS_TO_TICKS(timeout_ms));
  return received > 0 ? static_cast<std::size_t>(received) : 0;
}

}  // namespace radiance3d
