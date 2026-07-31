# Design decisions

Consequential decisions are recorded here until separate architecture decision
records become worthwhile.

| ID | Status | Decision | Reason |
| --- | --- | --- | --- |
| D-001 | Accepted | Use a monorepo. | Protocol, schema, firmware, and host changes can be reviewed together. |
| D-002 | Accepted | Keep motion and RF acquisition separate. | Receiver choice must not be embedded in motor firmware. |
| D-003 | Accepted | Use versioned JSON as the initial canonical format. | It is readable, extensible, and schema-validatable at current data volumes. |
| D-004 | Accepted | Use Python 3.11+ with no runtime dependency initially. | Typed models and useful validation do not yet require a framework. |
| D-005 | Accepted | Provide a native firmware simulator first. | Protocol behavior can be tested without claiming connected hardware. |
| D-006 | Provisional | Expose a line-oriented serial protocol. | It is easy to inspect during prototyping; framing may change after noise testing. |
| D-007 | Accepted | Rotate the AUT while the receiver/reference remains stationary. | Reduced receiver cable movement improves repeatability and cable strain while retaining a future slip-ring boundary. |
| D-008 | Accepted | Use degrees and the documented forward/right/up coordinate convention at public boundaries. | It keeps firmware configuration and scan files inspectable; radians remain internal to math utilities. |
| D-009 | Accepted | Treat Version 1 position as open-loop commanded position with explicit confidence. | Step counting is not independent verification; fault, reset, disable, stop, or suspected missed steps must force re-homing. |
| D-010 | Accepted | Preserve schema 1.0.0 reads while requiring schema 1.1.0 metadata for new scans. | Existing examples and external prototypes can migrate without weakening the new provenance contract. |
| D-011 | Accepted | Use native ESP-IDF v5.5.4 for physical motion firmware. | Explicit native task, timer, UART, watchdog, GPIO, and reset control are required while the portable C++ core remains testable. See [ADR](adr-native-esp-idf.md). |

New entries should state context, alternatives, consequences, and evidence. Changing
an accepted data or protocol contract requires a versioning and migration plan.
