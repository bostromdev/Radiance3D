#pragma once

#include <cstddef>
#include <cstdint>

namespace radiance3d {

// Keep input bias explicit in the portable configuration.  The ESP-IDF
// adapter maps these modes to gpio_config(); host fakes can model the same
// default levels without depending on ESP-IDF headers.
enum class PinMode { input, input_pullup, input_pulldown, output };

class HardwarePlatform {
 public:
  virtual ~HardwarePlatform() = default;

  virtual bool configure_pin(int pin, PinMode mode) = 0;
  virtual void write_pin(int pin, bool high) = 0;
  virtual bool read_pin(int pin) const = 0;
  virtual std::uint64_t monotonic_micros() const = 0;

  virtual bool begin_uart(std::uint8_t channel, int tx_pin, int rx_pin,
                          std::uint32_t baud) = 0;
  // TMC2209 PDN_UART may be wired as a single-wire bus.  The portable driver
  // requests this explicitly; platforms without a distinct mode can retain
  // their normal UART implementation and still filter write echo on receive.
  virtual bool configure_uart_half_duplex(std::uint8_t channel,
                                          bool enabled) {
    static_cast<void>(channel);
    static_cast<void>(enabled);
    return true;
  }
  virtual void flush_uart_input(std::uint8_t channel) = 0;
  virtual bool write_uart(std::uint8_t channel, const std::uint8_t* data,
                          std::size_t length) = 0;
  virtual std::size_t read_uart(std::uint8_t channel, std::uint8_t* data,
                                std::size_t maximum_length,
                                std::uint32_t timeout_ms) = 0;
};

}  // namespace radiance3d
