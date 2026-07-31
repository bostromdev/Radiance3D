#include "protocol.hpp"

#ifdef ARDUINO

#include "axis_controller.hpp"
#include "esp32_platform.hpp"
#include "hardware_config.hpp"
#include "physical_motion_controller.hpp"
#include "tmc2209_driver.hpp"

#include <Arduino.h>

namespace {

radiance3d::ArduinoEsp32Platform platform;
radiance3d::PhysicalControllerConfig physical_config =
    radiance3d::provisional_esp32_dev_config();
radiance3d::Tmc2209Driver azimuth_driver(
    platform, physical_config.azimuth.driver);
radiance3d::Tmc2209Driver elevation_driver(
    platform, physical_config.elevation.driver);
radiance3d::AxisController azimuth_axis(
    platform, azimuth_driver, physical_config.azimuth.axis);
radiance3d::AxisController elevation_axis(
    platform, elevation_driver, physical_config.elevation.axis);
radiance3d::PhysicalMotionController controller(
    platform, azimuth_axis, elevation_axis, physical_config);
radiance3d::ProtocolEngine engine(controller);
String incoming;
std::uint32_t last_host_activity_ms = 0;
bool host_seen = false;
bool host_watchdog_tripped = false;
constexpr std::uint32_t kHostWatchdogMs = 2000;

}  // namespace

void setup() {
  Serial.begin(115200);
  const radiance3d::GpioValidationResult gpio =
      radiance3d::validate_esp32_gpio(physical_config);
  if (gpio.bootstrapping_pin_mask != 0) {
    Serial.print("EVENT WARNING CODE=ESP32_BOOTSTRAP_GPIO MASK=");
    Serial.println(static_cast<unsigned long>(gpio.bootstrapping_pin_mask));
  }
  const bool initialized = controller.initialize();
  Serial.print("EVENT STARTUP READY=");
  Serial.print(initialized ? 1 : 0);
  Serial.print(" DRIVERS_ENABLED=0 BOARD=");
  Serial.println(physical_config.board_name);
}

void loop() {
  if (host_seen && !host_watchdog_tripped &&
      millis() - last_host_activity_ms >= kHostWatchdogMs &&
      (controller.state().azimuth.enabled ||
       controller.state().elevation.enabled)) {
    controller.stop();
    controller.set_enabled(false);
    host_watchdog_tripped = true;
    Serial.println(
        "EVENT FAULT CODE=DRIVER_DISABLED DETAIL=HOST_HEARTBEAT_TIMEOUT");
  }

  const std::string event = engine.service();
  if (!event.empty()) {
    Serial.println(event.c_str());
  }

  while (Serial.available() > 0) {
    const char character = static_cast<char>(Serial.read());
    if (character == '\n') {
      last_host_activity_ms = millis();
      host_seen = true;
      host_watchdog_tripped = false;
      Serial.println(engine.handle(incoming.c_str()).c_str());
      incoming = "";
    } else if (character != '\r') {
      if (incoming.length() < 255) {
        incoming += character;
      } else {
        incoming = "";
        Serial.println("ERR INVALID_ARGUMENT input line exceeds 255 bytes");
      }
    }
  }
}

#elif !defined(UNIT_TEST)

#include <iostream>
#include <string>

int main() {
  radiance3d::ProtocolEngine engine;
  std::string line;
  while (std::getline(std::cin, line)) {
    std::cout << engine.handle(line) << '\n';
    const std::string event = engine.service();
    if (!event.empty()) {
      std::cout << event << '\n';
    }
  }
  return 0;
}

#endif
