#pragma once

#include "hardware_platform.hpp"

#include <cstdint>

namespace radiance3d {

// ESP-IDF implementation of the driver-neutral physical I/O boundary.  It is
// deliberately limited to GPIO, TMC UARTs, and monotonic time; UART0 protocol
// ownership belongs to the protocol task, not this adapter.
class IdfHardwarePlatform final : public HardwarePlatform {
 public:
  bool configure_pin(int pin, PinMode mode) override;
  void write_pin(int pin, bool high) override;
  bool read_pin(int pin) const override;
  std::uint64_t monotonic_micros() const override;
  bool begin_uart(std::uint8_t channel, int tx_pin, int rx_pin,
                  std::uint32_t baud) override;
  bool configure_uart_half_duplex(std::uint8_t channel, bool enabled) override;
  void flush_uart_input(std::uint8_t channel) override;
  bool write_uart(std::uint8_t channel, const std::uint8_t* data,
                  std::size_t length) override;
  std::size_t read_uart(std::uint8_t channel, std::uint8_t* data,
                        std::size_t maximum_length,
                        std::uint32_t timeout_ms) override;

 private:
  static int uart_port(std::uint8_t channel);
  bool uart_installed_[3]{};
};

}  // namespace radiance3d
