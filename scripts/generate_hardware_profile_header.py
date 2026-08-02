#!/usr/bin/env python3
"""Validate the provisional hardware profile and emit a C++ configuration header.

The generated header is deliberately a build artifact.  Firmware defaults are not
maintained in a second handwritten C++ table: both the native ESP-IDF and host CMake
builds consume this output from the checked-in JSON profile.
"""

from __future__ import annotations

import argparse
import json
import math
import sys
from decimal import Decimal, InvalidOperation
from math import gcd
from pathlib import Path
from typing import Any


class ProfileError(ValueError):
    """Raised when a profile is not safe enough to compile into firmware."""


def _mapping(value: object, path: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise ProfileError(f"{path} must be an object")
    return value


def _int(value: object, path: str, *, minimum: int = 0, maximum: int | None = None) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise ProfileError(f"{path} must be an integer")
    if value < minimum or (maximum is not None and value > maximum):
        range_text = f">= {minimum}" if maximum is None else f"in {minimum}..{maximum}"
        raise ProfileError(f"{path} must be {range_text}")
    return value


def _number(value: object, path: str, *, positive: bool = False) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ProfileError(f"{path} must be a number")
    parsed = float(value)
    if not math.isfinite(parsed) or (positive and parsed <= 0.0):
        qualifier = "finite and positive" if positive else "finite"
        raise ProfileError(f"{path} must be {qualifier}")
    return parsed


def _bool(value: object, path: str) -> bool:
    if not isinstance(value, bool):
        raise ProfileError(f"{path} must be true or false")
    return value


def _string(value: object, path: str) -> str:
    if not isinstance(value, str) or not value.strip():
        raise ProfileError(f"{path} must be a non-empty string")
    return value


def _fraction(value: object, path: str) -> tuple[int, int]:
    try:
        decimal = Decimal(str(value))
    except (InvalidOperation, ValueError) as error:
        raise ProfileError(f"{path} must be a finite decimal") from error
    if not decimal.is_finite() or decimal <= 0:
        raise ProfileError(f"{path} must be finite and positive")
    sign, digits, exponent = decimal.as_tuple()
    if sign:
        raise ProfileError(f"{path} must be positive")
    numerator = 0
    for digit in digits:
        numerator = numerator * 10 + digit
    denominator = 1
    if exponent < 0:
        denominator = 10 ** (-exponent)
    else:
        numerator *= 10**exponent
    divisor = gcd(numerator, denominator)
    numerator //= divisor
    denominator //= divisor
    if numerator > 2**31 - 1 or denominator > 2**31 - 1:
        raise ProfileError(f"{path} cannot be represented as a 32-bit rational")
    return numerator, denominator


def _validate_input_pull(
    profile: dict[str, Any], key_prefix: str, path_prefix: str
) -> None:
    pullup = _bool(profile.get(f"{key_prefix}_pullup"), f"{path_prefix}_pullup")
    pulldown = _bool(profile.get(f"{key_prefix}_pulldown"), f"{path_prefix}_pulldown")
    if pullup and pulldown:
        raise ProfileError(f"{path_prefix} cannot enable both pull-up and pull-down")


def _axis(profile: dict[str, Any], name: str) -> dict[str, Any]:
    axis = _mapping(profile["axes"].get(name), f"axes.{name}")
    prefix = f"axes.{name}"
    required_numbers = (
        "minimum_angle_deg",
        "maximum_angle_deg",
        "home_offset_deg",
        "max_speed_deg_s",
        "acceleration_deg_s2",
        "home_speed_deg_s",
        "slow_home_speed_deg_s",
        "homing_backoff_deg",
    )
    for key in required_numbers:
        _number(axis.get(key), f"{prefix}.{key}", positive=key not in {"minimum_angle_deg", "maximum_angle_deg", "home_offset_deg"})
    if _number(axis["maximum_angle_deg"], f"{prefix}.maximum_angle_deg") <= _number(
        axis["minimum_angle_deg"], f"{prefix}.minimum_angle_deg"
    ):
        raise ProfileError(f"{prefix} maximum angle must exceed minimum angle")
    for key, upper in (
        ("uart_channel", 2),
        ("uart_address", 3),
        ("uart_tx_pin", 39),
        ("uart_rx_pin", 39),
        ("step_pin", 39),
        ("direction_pin", 39),
        ("enable_pin", 39),
        ("home_switch_pin", 39),
        ("motor_full_steps_per_revolution", 65535),
        ("microsteps", 256),
        ("commissioning_current_ma", 65535),
        ("maximum_rms_current_ma", 65535),
        ("hold_current_percent", 100),
        ("home_switch_debounce_ms", 60000),
        ("settling_time_ms", 60000),
        ("maximum_bench_test_steps", 2**31 - 1),
        ("motion_timeout_ms", 2**31 - 1),
    ):
        minimum = -1 if key == "home_switch_pin" else (0 if key == "uart_address" else 1)
        _int(axis.get(key), f"{prefix}.{key}", minimum=minimum, maximum=upper)
    if axis["commissioning_current_ma"] > axis["maximum_rms_current_ma"]:
        raise ProfileError(f"{prefix} commissioning current exceeds its safe ceiling")
    _string(axis.get("driver"), f"{prefix}.driver")
    _string(axis.get("driver_profile"), f"{prefix}.driver_profile")
    _bool(axis.get("direction_inverted"), f"{prefix}.direction_inverted")
    _bool(axis.get("home_switch_normally_closed"), f"{prefix}.home_switch_normally_closed")
    _bool(axis.get("homing_direction_negative"), f"{prefix}.homing_direction_negative")
    _validate_input_pull(axis, "home_switch", f"{prefix}.home_switch")
    numerator, denominator = _fraction(axis.get("gear_ratio"), f"{prefix}.gear_ratio")
    # One physical motor full step must map to an integral number of emitted
    # STEP pulses.  This prevents a valid-looking decimal ratio from silently
    # requiring fractional pulse counts at runtime.
    if (axis["microsteps"] * numerator) % denominator != 0:
        raise ProfileError(
            f"{prefix}.gear_ratio and microsteps must produce integral output steps"
        )
    return axis


def validate_profile(profile: dict[str, Any]) -> tuple[dict[str, Any], dict[str, Any], dict[str, Any]]:
    controller = _mapping(profile.get("controller"), "controller")
    _string(controller.get("board"), "controller.board")
    _string(controller.get("module"), "controller.module")
    _int(controller.get("protocol_version"), "controller.protocol_version", minimum=1, maximum=255)
    _int(controller.get("usb_serial_baud"), "controller.usb_serial_baud", minimum=1200, maximum=2_000_000)
    _int(controller.get("emergency_stop_pin"), "controller.emergency_stop_pin", minimum=0, maximum=39)
    _bool(controller.get("emergency_stop_active_low"), "controller.emergency_stop_active_low")
    _validate_input_pull(controller, "emergency_stop", "controller.emergency_stop")
    _int(
        controller.get("emergency_stop_debounce_ms"),
        "controller.emergency_stop_debounce_ms",
        minimum=1,
        maximum=60000,
    )

    drivers = _mapping(profile.get("drivers"), "drivers")
    _string(drivers.get("driver_family"), "drivers.driver_family")
    _int(drivers.get("uart_baud", 115200), "drivers.uart_baud", minimum=1200, maximum=2_000_000)
    _int(drivers.get("uart_timeout_ms", 20), "drivers.uart_timeout_ms", minimum=1, maximum=1000)
    _int(drivers.get("sense_resistor_milliohms", 110), "drivers.sense_resistor_milliohms", minimum=1, maximum=1000)
    _bool(drivers.get("enable_active_low", True), "drivers.enable_active_low")
    _bool(drivers.get("single_wire_pdn_uart", True), "drivers.single_wire_pdn_uart")
    _bool(drivers.get("write_echo_expected", True), "drivers.write_echo_expected")

    axes = _mapping(profile.get("axes"), "axes")
    if set(("azimuth", "elevation")) - set(axes):
        raise ProfileError("axes must contain azimuth and elevation")
    azimuth = _axis(profile, "azimuth")
    elevation = _axis(profile, "elevation")
    pins = [
        controller["emergency_stop_pin"],
        *[
            axis[key]
            for axis in (azimuth, elevation)
            for key in ("uart_tx_pin", "uart_rx_pin", "step_pin", "direction_pin", "enable_pin", "home_switch_pin")
        ],
    ]
    pins = [pin for pin in pins if pin >= 0]
    if len(pins) != len(set(pins)):
        raise ProfileError("controller and axis GPIO assignments must be unique")
    for axis in (azimuth, elevation):
        if axis["step_pin"] >= 34 or axis["direction_pin"] >= 34 or axis["enable_pin"] >= 34 or axis["uart_tx_pin"] >= 34:
            raise ProfileError("STEP/DIR/ENABLE/TX cannot use input-only ESP32 GPIOs")
    detector = _mapping(profile.get("detector"), "detector")
    adc_pin = _int(detector.get("adc_pin"), "detector.adc_pin", minimum=0, maximum=39)
    if adc_pin not in {32, 33, 34, 35, 36, 37, 38, 39}:
        raise ProfileError("detector.adc_pin must be an ADC1-capable ESP32 GPIO")
    if adc_pin in pins:
        raise ProfileError("detector ADC pin must not share a controller GPIO")
    power = _mapping(profile.get("power"), "power")
    if power.get("outputs_paralleled") is not False:
        raise ProfileError("buck converter outputs must not be paralleled")
    return controller, drivers, {"azimuth": azimuth, "elevation": elevation}


def _literal(value: str) -> str:
    return json.dumps(value, ensure_ascii=True)


def _boolean(value: bool) -> str:
    return "true" if value else "false"


def _floating(value: object) -> str:
    return f"{float(value):.12g}"


def _axis_initializer(name: str, axis: dict[str, Any]) -> str:
    numerator, denominator = _fraction(axis["gear_ratio"], "gear_ratio")
    values = (
        _literal(name),
        str(axis["uart_channel"]),
        str(axis["uart_address"]),
        str(axis["uart_tx_pin"]),
        str(axis["uart_rx_pin"]),
        str(axis["step_pin"]),
        str(axis["direction_pin"]),
        str(axis["enable_pin"]),
        str(axis["home_switch_pin"]),
        _boolean(axis["home_switch_normally_closed"]),
        _boolean(axis["home_switch_pullup"]),
        _boolean(axis["home_switch_pulldown"]),
        _boolean(axis["homing_direction_negative"]),
        str(axis["home_switch_debounce_ms"]),
        str(axis["motor_full_steps_per_revolution"]),
        str(axis["microsteps"]),
        str(axis["commissioning_current_ma"]),
        str(axis["maximum_rms_current_ma"]),
        str(axis["hold_current_percent"]),
        str(numerator),
        str(denominator),
        _boolean(axis["direction_inverted"]),
        _floating(axis["home_offset_deg"]),
        _floating(axis["minimum_angle_deg"]),
        _floating(axis["maximum_angle_deg"]),
        _floating(axis["max_speed_deg_s"]),
        _floating(axis["acceleration_deg_s2"]),
        _floating(axis["home_speed_deg_s"]),
        _floating(axis["slow_home_speed_deg_s"]),
        _floating(axis["homing_backoff_deg"]),
        str(axis["settling_time_ms"]),
        str(axis["maximum_bench_test_steps"]),
        str(axis["motion_timeout_ms"]),
    )
    return ",\n      ".join(values)


def render_header(controller: dict[str, Any], drivers: dict[str, Any], axes: dict[str, dict[str, Any]]) -> str:
    return f"""// Generated by scripts/generate_hardware_profile_header.py. Do not edit.
#pragma once

#include <cstdint>

namespace radiance3d {{
namespace generated_profile {{

struct AxisProfile {{
  const char* name;
  std::uint8_t uart_channel;
  std::uint8_t uart_address;
  int uart_tx_pin;
  int uart_rx_pin;
  int step_pin;
  int direction_pin;
  int enable_pin;
  int home_switch_pin;
  bool home_switch_normally_closed;
  bool home_switch_pullup;
  bool home_switch_pulldown;
  bool homing_direction_negative;
  std::uint32_t home_switch_debounce_ms;
  std::uint16_t motor_full_steps_per_revolution;
  std::uint16_t microsteps;
  std::uint16_t commissioning_current_ma;
  std::uint16_t maximum_rms_current_ma;
  std::uint8_t hold_current_percent;
  std::int32_t gear_ratio_numerator;
  std::int32_t gear_ratio_denominator;
  bool direction_inverted;
  double home_offset_deg;
  double minimum_angle_deg;
  double maximum_angle_deg;
  double maximum_speed_deg_per_s;
  double acceleration_deg_per_s2;
  double home_speed_deg_per_s;
  double slow_home_speed_deg_per_s;
  double homing_backoff_deg;
  std::uint32_t settling_time_ms;
  std::int64_t maximum_bench_test_steps;
  std::uint32_t motion_timeout_ms;
}};

constexpr const char kBoardName[] = {_literal(controller["board"])};
constexpr const char kBoardModule[] = {_literal(controller["module"])};
constexpr std::uint32_t kProtocolVersion = {controller["protocol_version"]}U;
constexpr std::uint32_t kHostUartBaud = {controller["usb_serial_baud"]}U;
constexpr int kEmergencyStopPin = {controller["emergency_stop_pin"]};
constexpr bool kEmergencyStopActiveLow = {_boolean(controller["emergency_stop_active_low"])};
constexpr bool kEmergencyStopPullup = {_boolean(controller["emergency_stop_pullup"])};
constexpr bool kEmergencyStopPulldown = {_boolean(controller["emergency_stop_pulldown"])};
constexpr std::uint32_t kEmergencyStopDebounceMs = {controller["emergency_stop_debounce_ms"]}U;
constexpr std::uint32_t kTmcUartBaud = {drivers.get("uart_baud", 115200)}U;
constexpr std::uint32_t kTmcUartTimeoutMs = {drivers.get("uart_timeout_ms", 20)}U;
constexpr std::uint16_t kTmcSenseResistorMilliohms = {drivers.get("sense_resistor_milliohms", 110)}U;
constexpr bool kTmcEnableActiveLow = {_boolean(drivers.get("enable_active_low", True))};
constexpr bool kTmcSingleWirePdnUart = {_boolean(drivers.get("single_wire_pdn_uart", True))};
constexpr bool kTmcWriteEchoExpected = {_boolean(drivers.get("write_echo_expected", True))};

constexpr AxisProfile kAzimuth = {{
      {_axis_initializer("azimuth", axes["azimuth"])}
}};

constexpr AxisProfile kElevation = {{
      {_axis_initializer("elevation", axes["elevation"])}
}};

}}  // namespace generated_profile
}}  // namespace radiance3d
"""


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--profile", type=Path, required=True)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--validate-only", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.validate_only and args.output is not None:
        print("--validate-only and --output cannot be combined", file=sys.stderr)
        return 2
    if not args.validate_only and args.output is None:
        print("--output is required unless --validate-only is used", file=sys.stderr)
        return 2
    try:
        profile = _mapping(json.loads(args.profile.read_text(encoding="utf-8")), "profile")
        controller, drivers, axes = validate_profile(profile)
    except (OSError, json.JSONDecodeError, ProfileError) as error:
        print(f"hardware profile validation failed: {error}", file=sys.stderr)
        return 1
    if args.validate_only:
        print(f"hardware profile valid: {args.profile}")
        return 0
    assert args.output is not None
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(render_header(controller, drivers, axes), encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
