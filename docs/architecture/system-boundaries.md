# System boundaries

## Motion boundary

The host sends positions in degrees and rates in degrees per second through the
versioned serial protocol. The controller reports its believed state and faults. A
future physical implementation must enforce machine-specific soft limits and
independent hardware safety; protocol range checks are not safety controls.

## Measurement boundary

A receiver adapter will return a numeric value, unit, sample timestamp, and device
metadata. RSSI is one possible value, not a required representation. Receiver setup,
settling, overload, and uncertainty remain device-specific.

## Dataset boundary

The host writes immutable raw scan records. Processing produces a new dataset with
`provenance.data_kind` set to `processed` and references to source dataset IDs.

## Presentation boundary

Analysis computes values; visualization displays them. A renderer must not silently
normalize, interpolate, or discard samples. Any such transformation belongs in a
documented processing step.
