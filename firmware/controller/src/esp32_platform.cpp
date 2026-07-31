#include "esp32_platform.hpp"

#ifdef ARDUINO

#include <esp_timer.h>

namespace radiance3d {

bool ArduinoEsp32Platform::configure_pin(const int pin, const PinMode mode) {
  if (pin < 0) {
    return false;
  }
  switch (mode) {
    case PinMode::input:
      pinMode(pin, INPUT);
      break;
    case PinMode::input_pullup:
      pinMode(pin, INPUT_PULLUP);
      break;
    case PinMode::output:
      pinMode(pin, OUTPUT);
      break;
  }
  return true;
}

void ArduinoEsp32Platform::write_pin(const int pin, const bool high) {
  digitalWrite(pin, high ? HIGH : LOW);
}

bool ArduinoEsp32Platform::read_pin(const int pin) const {
  return digitalRead(pin) == HIGH;
}

std::uint64_t ArduinoEsp32Platform::monotonic_micros() const {
  return static_cast<std::uint64_t>(esp_timer_get_time());
}

HardwareSerial* ArduinoEsp32Platform::uart(const std::uint8_t channel) const {
  if (channel == 1) {
    return &Serial1;
  }
  if (channel == 2) {
    return &Serial2;
  }
  return nullptr;
}

bool ArduinoEsp32Platform::begin_uart(const std::uint8_t channel,
                                      const int tx_pin, const int rx_pin,
                                      const std::uint32_t baud) {
  HardwareSerial* serial = uart(channel);
  if (serial == nullptr) {
    return false;
  }
  serial->begin(baud, SERIAL_8N1, rx_pin, tx_pin);
  return true;
}

void ArduinoEsp32Platform::flush_uart_input(const std::uint8_t channel) {
  HardwareSerial* serial = uart(channel);
  if (serial == nullptr) {
    return;
  }
  while (serial->available() > 0) {
    serial->read();
  }
}

bool ArduinoEsp32Platform::write_uart(const std::uint8_t channel,
                                      const std::uint8_t* data,
                                      const std::size_t length) {
  HardwareSerial* serial = uart(channel);
  return serial != nullptr && serial->write(data, length) == length;
}

std::size_t ArduinoEsp32Platform::read_uart(
    const std::uint8_t channel, std::uint8_t* data,
    const std::size_t maximum_length, const std::uint32_t timeout_ms) {
  HardwareSerial* serial = uart(channel);
  if (serial == nullptr) {
    return 0;
  }
  const std::uint64_t started = monotonic_micros();
  const std::uint64_t timeout_us =
      static_cast<std::uint64_t>(timeout_ms) * 1000ULL;
  std::size_t received = 0;
  while (received < maximum_length &&
         monotonic_micros() - started < timeout_us) {
    while (serial->available() > 0 && received < maximum_length) {
      data[received++] = static_cast<std::uint8_t>(serial->read());
    }
    if (received >= 8) {
      break;
    }
    yield();
  }
  return received;
}

}  // namespace radiance3d

#endif
