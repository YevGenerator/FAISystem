# FAISystem (Feeling Artificial Intelligence System)

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B20)

**FAISystem** is a distributed C++20 execution engine and toolchain designed for **Feeling Artificial Intelligence (Feeling AI)** networks.

---


## System Modules

The repository follows the next layer separation:

| Module | Description | Dependencies |
| :--- | :--- | :--- |
| [`Core/`](./Core) | Mathematical and domain core: knowledge granules (`Kg...`, `Node`), `2SNO` algorithm, timers, events and binary structures. | *None (Standard C++20)* |
| [`strings/`](./strings) | Lexical and AST toolkit: string parsers and type converters. | `Core` |
| [`ZMCore/`](./ZMCore) | Distributed networking runtime: ZeroMQ message bus (`ZMBus`), worker thread pools, and routing dispatcher (`ZMRouter`). | `Core`, `cppzmq` |
| [`build_compiler/`](./build_compiler) | Standalone CLI tools: configuration compiler (`rpcompiler`) and decompiler (`rpdecompiler`). | `Core`, `strings` |
| [`cli_serv/`](./cli_serv) | Host runtime executables: server and clients (`servant`) applications. | `Core`, `ZMCore` |
| [`visualizer/`](./visualizer) | Monitoring UI for inspection of the system's work via binary log deserialization. | `Core`, `imgui` |
| [`lsp_server/`](./lsp_server) | Language Server Protocol (LSP) daemon for real-time diagnostics and syntax highlighting for IDEs. | `Core`, `strings`, `nlohmann_json` |

---

## Documentation and User Guides

Detailed documentation, architecture specifications, and guides are organized in the [`docs/`](./docs) directory:

- **[Quick Start: Configuration Compiler Guide](./docs/compiler.md)**  
  *Step-by-step tutorial on building tools, writing `.rpconfig` scripts, compiling to `.bin`, and environment verification.*
- **[Quick Start: Running CLI Executables Guide](./docs/cli_serves.md)**  
  *Deployment, launching and using programs of `cli_serv` executables*

---

## Quick Usage Guide

### 1. Download Release Binaries
Get the latest pre-compiled static executables from the [Releases](https://github.com/YevGenerator/FAISystem/releases) page:
- `rpcompiler` (or `rpcompiler.exe`)
- `rpdecompiler` (or `rpdecompiler.exe`)
- `server` (or `server.exe`)
- `servant` (or `servant.exe`)

### 2. Prepare Network Topology Script (`network.rpconfig`)
Create a declarative configuration script defining knowledge granules, inputs, sensors, and routing:

```rpconfig
# 1. Constants and input definitions
let KgInput defaultSlot = <c_i="up" a_i=0.7500 b_i=0 v_i=0.1000 g_i=0.5000>
let []KgInput nodeInputs = [
    defaultSlot
    <c_i="down" a_i=0.3000 b_i=1 v_i=0.1500 g_i=0.8500>
]

# 2. Host networking setup
device 1
workers 2
ip <host=127.0.0.1 port=7779>

# 3. Sensory input and knowledge granule
sensor <deviceId=1 id=10>
node <deviceId=1 id=1.0 algo="2SNO" output=<c="up" a=0.8500> inputs=nodeInputs>

# 4. External binding and trigger
bindE <from=<deviceId=1 id=1.0> to=[ <deviceId=2 id=2.0 in=0> ]>
run <deviceId=1 toRun=true>
```

### 3. Compile Configuration to Binary Image
Translate human-readable DSL into a flat, zero-parser binary package (`.bin`):

```bash
# Direct CLI compilation (generates network.bin automatically)
./rpcompiler network.rpconfig

# Or run interactively (prompt will request input/output paths)
./rpcompiler
```

### 4. Launch Distributed Runtime Hosts
Execute coordinator and worker services with the compiled binary configuration:

```bash
# Start the central coordinator host
./server network.bin

# Start servant computing nodes (on target remote/local machines)
./servant 2 network.bin
```

---

## Quick Build Guide

### Prerequisites
- C++ compiler with full **C++20** support (GCC 13+, Clang 16+, MSVC 2022);
- **CMake** 3.20 or higher;
- **ZeroMQ** and **cppzmq** development packages (e.g., via `vcpkg` or system package manager).

### Build from Source
```bash
# 1. Clone the repository
git clone [https://github.com/YevGenerator/FAISystem.git](https://github.com/YevGenerator/FAISystem.git)
cd FAISystem

# 2. Configure build environment
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 3. Build compiler utilities and runtime engines
cmake --build build --config Release