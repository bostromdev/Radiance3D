#include "protocol.hpp"

#ifdef ARDUINO
#include <Arduino.h>

radiance3d::ProtocolEngine engine;
String incoming;

void setup() { Serial.begin(115200); }

void loop() {
  while (Serial.available() > 0) {
    const char character = static_cast<char>(Serial.read());
    if (character == '\n') {
      Serial.println(engine.handle(incoming.c_str()).c_str());
      incoming = "";
    } else if (character != '\r') {
      incoming += character;
    }
  }
}
#else
#include <iostream>
#include <string>

int main() {
  radiance3d::ProtocolEngine engine;
  std::string line;
  while (std::getline(std::cin, line)) {
    std::cout << engine.handle(line) << '\n';
  }
  return 0;
}
#endif
