# Motion controller

The physical ESP32 target is a native ESP-IDF v5.5.4 project. Portable C++ core code
and the simulator build independently with host CMake. The physical wiring, carrier
revision, PDN_UART topology, pulse timing, and bring-up results remain pending
validation.

## ESP-IDF build and flash

Install the pinned ESP-IDF release using Espressif's documented setup, then export its
environment in each shell:

```bash
git clone -b v5.5.4 --recursive https://github.com/espressif/esp-idf.git ~/esp/esp-idf-v5.5.4
~/esp/esp-idf-v5.5.4/install.sh esp32
. ~/esp/esp-idf-v5.5.4/export.sh

cd firmware/controller
idf.py set-target esp32
idf.py build
idf.py size
idf.py flash
idf.py monitor
```

UART0 is the line-oriented host protocol. Console logging is disabled so `idf.py
monitor` should not be used as a debug-log console on a connected controller; use the
host protocol or a separate diagnostics path. `idf.py monitor` can still observe the
protocol line and any early ROM output.

## Host simulator and portable tests

```bash
cd firmware/controller
cmake -S host -B build-host -DCMAKE_BUILD_TYPE=Release
cmake --build build-host
ctest --test-dir build-host --output-on-failure
./build-host/radiance3d-simulator
```

The simulator reads one command per line from standard input and writes responses and
events to standard output. See [the protocol specification](../../docs/firmware/protocol.md)
and [native architecture](../../docs/firmware/esp-idf-architecture.md).
