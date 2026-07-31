# Firmware tests

Portable CTest executables exercise configuration-derived angular conversion, rational
gearing, configured travel limits, homing, trust loss, driver-disable behavior,
TMC2209 CRC/IFCNT/write-echo handling, GPIO validation, e-stop latching, coordinated
completion, and protocol synchronization.

```bash
cmake -S ../host -B ../build-host -DCMAKE_BUILD_TYPE=Release
cmake --build ../build-host
ctest --test-dir ../build-host --output-on-failure
```

ESP-IDF CI compiles the physical project with `idf.py build`. Target-side tests require
an ESP32 test device and are not presented as a substitute for commissioning. Python CI
also runs the unchanged serial host client against the CMake-built simulator. Physical
test records must state board revision, wiring, load, supply, measurement instrument,
and safety controls.
