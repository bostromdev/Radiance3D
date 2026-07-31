# System boundaries

## Motion boundary

The host sends positions in degrees and rates in degrees per second through the
versioned serial protocol. The controller reports its believed state and faults. A
future physical implementation must enforce machine-specific soft limits and
independent hardware safety; protocol range checks are not safety controls.

The host-facing `MotionController` protocol is transport-neutral. The firmware-facing
`MotionController` interface owns homing, limits, position confidence, enable, stop,
and emergency-stop behavior; a TMC2209 implementation must stay behind that boundary.
Reported Version 1 position is open-loop commanded position, never independently
verified physical position.

## Measurement boundary

A receiver adapter returns a numeric value, native unit, sample timestamp, source
identifier, validity state, and optional warnings. RSSI is one possible value, not a
required representation. Receiver setup, settling, overload, and uncertainty remain
device-specific. The motion controller never calls this interface.

## Dataset boundary

The host writes immutable raw scan records. Processing produces a new dataset with
`provenance.data_kind` set to `processed` and references to source dataset IDs.

## Presentation boundary

Analysis computes values; visualization displays them. A renderer must not silently
normalize, interpolate, or discard samples. Any such transformation belongs in a
documented processing step.
