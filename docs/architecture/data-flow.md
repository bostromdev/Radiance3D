# Data flow

One planned measurement sample moves through the system as follows:

```mermaid
sequenceDiagram
    participant H as Host coordinator
    participant M as Motion controller
    participant R as Receiver adapter
    participant D as Dataset writer

    H->>M: SCAN_STEP azimuth elevation rate
    M-->>H: READY with reported position
    H->>R: Request measurement
    R-->>H: value, unit, timestamp, device status
    H->>D: Append sample + angle + flags
    D-->>H: Persisted or explicit error
```

The controller's `READY` response is a synchronization boundary in the simulator, not
proof that a physical axis has settled. A physical implementation may need encoder
feedback, dwell time, vibration criteria, and receiver settling. The host must record
reported rather than merely commanded angles when that information is available.

Write failures must stop or pause a scan rather than allow unrecorded measurements.
