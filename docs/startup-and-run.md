# Runtime lifecycle: server and servant
## Invocation

Each process accepts its **own** compiled binary configuration:

```powershell
.\servantZMQ.exe .\servant-2.bin
.\servantZMQ.exe .\servant-3.bin
.\serverZMQ.exe .\server.bin
```

The servant configuration needs `device`, `workers` and `ip`; the server configuration additionally defines the topology (sensors, nodes, routes) and `run` commands. The server remains the source of topology configuration.

## Command lifecycle

- `device` assigns the local device identity; `workers` builds worker objects but does not launch them.
- The first `ip` starts the **ZMBus thread** and waits until the local ROUTER bind or DEALER connect socket setup has completed. ZeroMQ DEALER connection establishment is asynchronous; local socket readiness is not a remote handshake.
- Following config commands are processed locally when addressed to this device and queued for remote dispatch when addressed to another device.
- A ROUTER learns a servant ID from its DEALER probe (enabled by `probe_router`). Pending configuration messages for that ID are sent in FIFO order from the ZMBus thread. The configuration loader must never directly use the ZMQ socket.
- `run <deviceId=... toRun=true>` starts the local router, workers and sensor thread on the addressed process. `toRun=false` stops them. The ZMBus thread stays alive for configuration and network traffic while computation is stopped.
- Repeating `ip` is currently rejected: changing the endpoint of a running ZeroMQ socket requires a separate reconfiguration design.

