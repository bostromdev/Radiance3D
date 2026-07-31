#include "protocol.hpp"

#include <chrono>
#include <iostream>
#include <string>

#if defined(__unix__) || defined(__APPLE__)
#include <poll.h>
#include <unistd.h>
#endif

namespace {

constexpr std::chrono::milliseconds kPollInterval{20};
constexpr std::chrono::seconds kHostHeartbeatTimeout{2};

bool stdin_ready() {
#if defined(__unix__) || defined(__APPLE__)
  pollfd input = {};
  input.fd = STDIN_FILENO;
  input.events = POLLIN;
  return poll(&input, 1, static_cast<int>(kPollInterval.count())) > 0;
#else
  return true;
#endif
}

void print_line(const std::string& line) {
  std::cout << line << '\n';
  std::cout.flush();
}

}  // namespace

int main() {
  radiance3d::ProtocolEngine engine;
  std::string line;
  bool host_seen = false;
  bool heartbeat_tripped = false;
  auto last_host_activity = std::chrono::steady_clock::now();
  for (;;) {
    if (stdin_ready()) {
      if (!std::getline(std::cin, line)) {
        break;
      }
      last_host_activity = std::chrono::steady_clock::now();
      host_seen = true;
      heartbeat_tripped = false;
      print_line(engine.handle(line));
    }
    const auto now = std::chrono::steady_clock::now();
    if (host_seen && !heartbeat_tripped &&
        (engine.state().azimuth.enabled || engine.state().elevation.enabled) &&
        now - last_host_activity >= kHostHeartbeatTimeout) {
      print_line(engine.host_heartbeat_timeout());
      heartbeat_tripped = true;
    }
    const std::string event = engine.service();
    if (!event.empty()) {
      print_line(event);
    }
  }
  return 0;
}
