# Assembly, cable routing, and commissioning

## Assembly order

1. Print and inspect the complete mount.
2. Install both YEJMKJ motors.
3. Install the ESP32, both TMC2209 modules, both ZX-052 modules, and AD8317 in their
   designed positions; compare orientation, connectors, and heatsinks to the photo library.
4. Provide the accessible, strain-relieved `+12V IN` / `GND IN` enclosure entry and
   terminate PWR-000 at the internal distribution point; do not install or allocate a battery.
5. Move pan and tilt manually through the intended ranges.
6. Route string or sacrificial silicone wire for PWR/SIG/MTR paths and sacrificial
   RG316 for RF paths; add the intended service loops and record safe final lengths
   before cutting any final harness.
7. Cut, terminate, label, continuity-test, and strain-relieve the final harnesses.
8. Set and meter both buck outputs at 5.0 V without electronics connected.
9. Connect electronics, inspect again, then perform powered motion testing.

## Routing requirement

**Design Intent — conceptual architecture only. Final placement determined during CAD.**
The moving silicone harness is intended to carry only tilt-motor and AD8317 electrical
wiring across the pan axis. A controlled 360° turn, service loop, strain relief, cable
guide, bend radius, and return strategy are CAD/commissioning requirements—not known
geometry. The external VTX and its RG316 path are stationary and off-scanner.

Unlimited continuous pan rotation is not allowed. The intended mechanical capability is
one controlled 360° turn. Firmware limits are not changed by this documentation baseline
and remain provisional pending physical test. The required future control concept is the
**Firmware-defined return-to-home strategy to prevent cumulative cable twist**; its
implementation remains deferred.

After an electrical test, a printed clip, cable tie, clamp, or removable hot-glue strain
relief is preferred. Adhesive must not enter connectors, adjustment points, heatsinks,
or moving surfaces; permanent cyanoacrylate is not structural retention for a serviceable
connector.

## Commissioning checklist

- [ ] Compare each installed board and connector orientation to its repository photo.
- [ ] Verify no raw 12 V path reaches ESP32 5V/VIN, AD8317 +5V, or any GPIO.
- [ ] Confirm PWR-000 enters through an accessible strain relief and terminates at the
      internal distribution point; confirm no battery compartment exists.
- [ ] Meter Buck A and Buck B independently at 5.0 V; confirm their VOUT+ terminals are not joined.
- [ ] Verify common ground continuity and motor phase mapping end to end.
- [ ] Confirm every harness has labels at both ends and a matching table row.
- [ ] Confirm RF-001 is the off-scanner VTX coax path and that no RF path is shown or
      built with silicone wire.
- [ ] Confirm the AUT directly threads onto the AD8317 SMA with no RG316 jumper.
- [ ] Confirm all PWR/SIG/MTR harnesses are silicone wire, never RG316.
- [ ] Build and run simulator/repository checks before flashing.
- [ ] Verify PDN/UART communication and current settings with motors disconnected.
- [ ] Install heatsinks, connect motors while unpowered, then make a low-speed direction test.
- [ ] Verify one controlled 360° pan turn and the future Firmware-defined return-to-home
      strategy to prevent cumulative cable twist after CAD and physical testing.
- [ ] Capture AD8317 raw ADC and voltage data; do not report dBm without calibration.

## Failure modes and troubleshooting

| Symptom | Safe first checks |
|---|---|
| ESP32 will not boot | remove motor power; check that only regulated 5 V reaches VIN and bootstrap pins are untouched |
| Motor buzzes or locks | power off; check A1→Black, A2→Yellow/Green, B1→Red, B2→Blue continuity |
| Motor skips or driver overheats | reduce current/load; verify heatsink and motion acceleration; do not increase above 800 mA RMS |
| UART diagnostics fail | power off; check PDN pin identity, address 0/1, UART topology, and common ground |
| ADC reading is unstable | separate RF/analog harness from VM/phase wiring; confirm RF 5 V and analog ground |
| Cable rubs or tightens | stop immediately; revise routing and measured soft limits before further motion |

Unresolved bench measurements: rear OLED pin ownership; AD8317 input/output connector
labels and calibration curve; ZX-052 electrical limits; final mount travel; final cable
lengths; driver PDN/UART topology; and all physical clearances. None may be guessed.
