# Native ESP-IDF firmware troubleshooting

## The host client receives non-protocol text

UART0 is reserved for the structured protocol. Confirm
`CONFIG_ESP_CONSOLE_UART_NONE=y` is present and do not enable serial logs on UART0.
Some ESP32 boot ROM messages can precede application startup; reset the serial input
buffer before `IDENTIFY` and do not parse those messages as protocol lines.

## `EVENT STARTUP READY=0`

Keep motor power disconnected. Inspect the reported board/profile, validate the JSON
profile, check duplicate/boot-strap pins, then verify each TMC UART path and carrier
pinout. The controller intentionally leaves both drivers disabled on this condition.

## TMC2209 communication fault or absent driver

Confirm common ground, UART1/UART2 assignment, RX/TX/PDN_UART topology, address
straps, the exact carrier R10/sense resistor, and 3.3 V logic compatibility. The
single-wire PDN_UART profile is provisional; do not assume a carrier exposes the same
electrical circuit as another board revision.

## Host heartbeat timeout

The host must send a command or `HEARTBEAT` at least every two seconds while a driver
is enabled. The Python client does this while waiting for asynchronous motion. A
timeout stops and disables drivers and invalidates position; inspect the host/USB link,
then home again.

## E-stop cannot clear

Release the physical input, wait for debounce, resolve the hazard, then issue
`CLEAR_FAULT` or `RESET_ESTOP`. Firmware cannot clear a latched e-stop while the input
is active. Use the physical motor-power disconnect when needed.

## Unexpected reset or `RESET=BROWNOUT`

Drivers are disabled and position is untrusted after every reset. Rehome before any
absolute move. Inspect supply wiring, LM2596 adjustment, USB backfeed, bulk capacitance,
and motor load with appropriate instruments; a buck's front-panel voltage is not a
transient measurement.

## Build failures

Use ESP-IDF v5.5.4 and run the shell export script before `idf.py`. Remove only the
project's generated `firmware/controller/build/` directory when changing target or
toolchain; do not delete source profile or hardware records. Portable tests use CMake
under `firmware/controller/host` and do not require ESP-IDF.
