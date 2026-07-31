#pragma once

#include <cstddef>
#include <cstdint>

namespace radiance3d {

enum class PinMode { input, input_pullup, output };

class HardwarePlatform {
 public:
  virtual ~HardwarePlatform() = default;

  virtual bool configure_pin(int pin, PinMode mode) = 0;
  virtual void write_pin(int pin, bool high) = 0;
  virtual bool read_pin(int pin) const = 0;
  virtual std::uint64_t monotonic_micros() const = 0;

  virtual bool begin_uart(std::uint8_t channel, int tx_pin, int rx_pin,
                          std::uint32_t baud) = 0;
  virtual void flush_uart_input(std::uint8_t channel) = 0;
  virtual bool write_uart(std::uint8_t channel, const std::uint8_t* data,
                          std::size_t length) = 0;
  virtual std::size_t read_uart(std::uint8_t channel, std::uint8_t* data,
                                std::size_t maximum_length,
                                std::uint32_t timeout_ms) = 0;
};

}  // namespace radiance3d
