#pragma once

#include "hardware_platform.hpp"

#ifdef ARDUINO

#include <Arduino.h>

namespace radiance3d {

class ArduinoEsp32Platform final : public HardwarePlatform {
 public:
  bool configure_pin(int pin, PinMode mode) override;
  void write_pin(int pin, bool high) override;
  bool read_pin(int pin) const override;
  std::uint64_t monotonic_micros() const override;
  bool begin_uart(std::uint8_t channel, int tx_pin, int rx_pin,
                  std::uint32_t baud) override;
  void flush_uart_input(std::uint8_t channel) override;
  bool write_uart(std::uint8_t channel, const std::uint8_t* data,
                  std::size_t length) override;
  std::size_t read_uart(std::uint8_t channel, std::uint8_t* data,
                        std::size_t maximum_length,
                        std::uint32_t timeout_ms) override;

 private:
  HardwareSerial* uart(std::uint8_t channel) const;
};

}  // namespace radiance3d

#endif
