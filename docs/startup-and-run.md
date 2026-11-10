# Runtime lifecycle: server and servant

These notes apply to `fix/logging-visualizer-runtime-20261108`.

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

## Important limits

A ready local TCP socket does **not** imply the servant has received or applied its configuration. The current route-probe mechanism permits FIFO dispatch but does not yet provide end-to-end CONFIG_ACK acknowledgements. Therefore, local server sensors may start before all remote devices have fully applied their commands. To guarantee a simultaneous distributed start, introduce an acknowledgement/barrier before the server activates its local `run`.

The in-memory pending queues are not a durable message store. Restarting the server discards queued commands; reconnect handling and config replay should be tested before relying on fault tolerance.

The Windows CI separately builds the CLI targets and runs the queue smoke tests before building the visualizer.
