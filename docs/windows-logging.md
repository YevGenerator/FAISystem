# Windows 11: build, run and inspect logs

These instructions target **Visual Studio 2022** (MSVC C++20), **CMake 3.20+**, **vcpkg**, and PowerShell. All paths are relative to the cloned repository unless specified.

## Build

1. Install the C++ Desktop development workload in Visual Studio 2022, Git, CMake and vcpkg.
2. Set `VCPKG_ROOT` to your vcpkg checkout. The repository's `vcpkg.json` declares cppzmq/ZeroMQ, GLFW, ImGui (GLFW and OpenGL 3 backends) and nlohmann-json.

```powershell
git switch fix/logging-visualizer-runtime-20261108
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 "-DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Use the actual `VCPKG_ROOT` location on your machine. CMake target names: `rpcompiler`, `rpdecompiler`, `serverZMQ`, `servantZMQ`, `RPVisualizer`.

## Run three processes

Run the following commands in separate PowerShell terminals. Substitute your own compiled configuration and executable paths. **Every process must load the same `network.bin`**, and the servant IDs must match the `device` values used in the configuration.

```powershell
.\build\build_compiler\Release\rpcompiler.exe .\network.rpconfig .\network.bin
.\build\cli_serv\Release\serverZMQ.exe .\network.bin --log .\server.log.bin
.\build\cli_serv\Release\servantZMQ.exe .\network.bin -d 2 --log .\servant-2.log.bin
.\build\cli_serv\Release\servantZMQ.exe .\network.bin -d 3 --log .\servant-3.log.bin
```

For backward compatibility, servant also accepts `servantZMQ.exe 2 network.bin`. If `--log` is omitted, each process chooses an unused file such as `server.log.bin`, `servant.log.bin` or `servant-1.log.bin`. Logs are **binary**, not text.

When you have emitted some sensor data, open the logs in a separate terminal:

```powershell
.\build\ui\visualizer\Release\RPVisualizer.exe .\server.log.bin .\servant-2.log.bin .\servant-3.log.bin
```

Without CLI arguments, the visualizer reads all `*.log.bin` files in its current working directory. For traces spanning multiple devices, supply **all process files from the same run**. Paths may be quoted if they contain spaces. Close the running processes cleanly after collecting enough data.

## Log event model

Every binary record has the same packed prefix: `logType (u8)`, `EventHeader (eventId, parentId, traceId)`, `elapsed microseconds (u64)`, then the typed payload. See `Core/logger/LogRecordList.hpp` for the stable per-type ordinals and `Core/logger/LogRecordTypes.hpp` for payload layouts.

- Configuration events record device ID, worker count, IP, sensors, nodes, inputs, binds and run controls.
- Sensor output is a root event.
- The router creates child events for accept/inner-routing/forward-routing.
- A worker creates child events for input acceptance, node evaluation and output.
- TCP transit carries the event header unchanged; the receiving router creates the next child event.

The visualizer stitches events from all files by `(deviceId, localEventId)` and uses `traceId` to group their causal tree. Do not concatenate log files manually: give their paths to the visualizer.

## Notes

This branch includes `tests/logging_smoke.cpp` and a Windows CI workflow. A successful end-to-end distributed trace still requires a real run with several Windows processes; the test alone does not validate ZeroMQ network behavior.
