# Roadmap

Remaining work only. A stage is done when its result is reproducible, not when the code
compiles.

## Done

Firmware, simulator, host client, protocol, motion state machine, homing logic,
emergency stop, tests. Hardware measured and photographed — see
[`Measurements/`](Measurements/README.md).

## Now — get it moving

Nothing here needs a printed part. This is the fastest path to a working machine.

- [ ] USB-only ESP32 bring-up. Motor power disconnected.
- [ ] Confirm GPIO map against the actual board — see [`HARDWARE.md`](HARDWARE.md).
- [ ] TMC2209 UART comms, read diagnostics back.
- [ ] One motor spinning on the bench at 650 mA RMS. Coil A black+green, B red+blue.
- [ ] Both axes. Direction, acceleration, e-stop.
- [ ] Thermal check under sustained motion.

## Next — the mechanism

- [ ] Measure the parts that block CAD: bearings, couplers, fasteners, heat-set
      inserts, antenna mount, NEMA 17 mounting-hole pattern. See
      [`missing-measurements.md`](Measurements/missing-measurements.md).
- [ ] Decide homing: skip it, StallGuard, or switches. See `HARDWARE.md`.
- [ ] Print the register fit coupon. Confirm `dPrintClearance` for PETG.
- [ ] Design `Pan_Base` first, one component at a time.
- [ ] Print, assemble, check the antenna sits on the pan/tilt axis intersection.

## Then — actually measuring

- [ ] Wire the AD8317 and read it from the host.
- [ ] First full scan. Raw data with complete provenance.
- [ ] Repeatability: same antenna twice, compare.
- [ ] Plot it.

## Later — earning the accuracy claim

Reference antenna, uncertainty budget, comparison against trusted equipment. Only after
this can any accuracy number be stated.

## Video

The first bench milestone — motors turning under their own firmware — is filmable and
does not need any of the mechanism. Do that before printing anything.
