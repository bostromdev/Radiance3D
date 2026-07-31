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

New entries should state context, alternatives, consequences, and evidence. Changing
an accepted data or protocol contract requires a versioning and migration plan.
