# Data flow

One planned measurement sample moves through the system as follows:

```mermaid
sequenceDiagram
    participant H as Host coordinator
    participant M as Motion controller
    participant R as Receiver adapter
    participant D as Dataset writer

    H->>M: SCAN_STEP azimuth elevation rate
    M-->>H: READY with trusted commanded position
    H->>H: Apply configured settling delay
    loop Samples per position + retries
        H->>R: Request measurement with timeout
        R-->>H: native value, unit, timestamp, source, validity, warnings
        H->>D: Append raw accepted or rejected reading
    end
    D-->>H: Persisted or explicit error
```

The controller's `READY` response is a synchronization boundary in the simulator, not
proof that a physical axis has settled. A physical implementation may need encoder
feedback, dwell time, vibration criteria, and receiver settling. The host must record
the controller's position kind; Version 1 reports commanded/open-loop position.

Write failures must stop or pause a scan rather than allow unrecorded measurements.
