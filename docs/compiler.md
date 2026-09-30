# Quick Installation & Usage Guide (Feeling AI Compiler)

### Overview
`rpcompiler` is a command-line tool designed for **FAISystem**. It compiles high-level declarative Feeling AI network topology scripts (`.rpconfig`) into flat, atomic binary images (`.bin`).

---

### Step 1: Obtain the Compiler Executables

#### Option A: Download Pre-built Binaries (Recommended)
1. Go to the repository releases page: [FAISystem Releases](https://github.com/YevGenerator/FAISystem/releases).
2. Choose the latest stable release (e.g., tag `v1.0.0`).
3. Under the **Assets** section, download standalone binaries for your OS:
    - `rpcompiler.exe` (or `rpcompiler` on Linux) — Configuration compiler.
    - `rpdecompiler.exe` (or `rpdecompiler` on Linux) — Binary reverse decompiler.

#### Option B: Build from Source via CMake
```bash
# Clone the repository
git clone [https://github.com/YevGenerator/FAISystem.git](https://github.com/YevGenerator/FAISystem.git)
cd FAISystem

# Configure CMake build directory
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build compiler and decompiler targets
cmake --build build --target rpcompiler rpdecompiler --config Release
```
*Compiled binaries will be placed in `build/build_compiler/`.*

---

### Step 2: Prepare a Configuration Script (`test_node.rpconfig`)

Create a configuration file describing the desired network topology (knowledge granules, inputs, sensors, and inter-device routing):

```rpconfig
# ==========================================================
# Feeling AI Network Configuration
# ==========================================================

# 1. Compile-time constants
let uint32 devHost = 1
let uint32 devRemote = 2

# Input slots for knowledge granule: <c_i a_i b_i v_i g_i>
let KgInput defaultSlot = <c_i="up" a_i=0.7500 b_i=0 v_i=0.1000 g_i=0.5000>
let []KgInput nodeInputs = [
    defaultSlot
    <c_i="down" a_i=0.3000 b_i=1 v_i=0.1500 g_i=0.8500>
]

# 2. Host networking setup
device devHost
workers 2
ip <host=127.0.0.1 port=7779>

# 3. Sensory input and knowledge granule (2SNO algorithm)
sensor <deviceId=devHost id=10>
node <deviceId=devHost id=1.0 algo="2SNO" output=<c="up" a=0.8500> inputs=nodeInputs>

# 4. External route binding
bindE <from=<deviceId=devHost id=1.0> to=[ <deviceId=devRemote id=2.0 in=0> ]>

# 5. Runtime activation trigger
run <deviceId=devHost toRun=true>
```

---

### Step 3: Compile Configuration (`.rpconfig` -> `.bin`)

Run `rpcompiler` to generate the packed binary configuration file:

```bash
# Mode 1: Direct compile (automatically generates test_node.bin)
./rpcompiler test_node.rpconfig

# Mode 2: Explicit output file path specification
./rpcompiler test_node.rpconfig network.bin

# Mode 3: Interactive TUI mode (run without parameters or via double-click)
./rpcompiler
# The prompt will ask:
# "Enter input file path (.rpconfig): " -> Provide path or drag-and-drop file
# "Enter output file path [.bin]: "     -> Press Enter to accept default
```

#### What happens during compilation:
- All `let` constants are resolved and inlined into the corresponding AST fields.
- Composite `node` declarations are atomized into a single `CmdNodeCreate` instruction and sequential `CmdNodeAddInput` calls for each input slot.
- Route declarations (`bindE`) are expanded into flat `CmdBindDouble` binary records.
- All commands are serialized using packed C++ POD structures (`#pragma pack(push, 1)`).

---

### Step 4: Environment Verification & Decompilation

To verify the correctness, memory alignment, and parameter integrity of the generated binary file, run `rpdecompiler`:

```bash
# Decompile the binary image back to readable DSL
./rpdecompiler test_node.bin decompiled.rpconfig
```

Inspect the reconstructed configuration:
```bash
# View result on Linux / macOS
cat decompiled.rpconfig

# View result on Windows PowerShell
Get-Content decompiled.rpconfig
```

If the decompiled topology accurately reflects the original node definitions, thresholds, and routing paths, the `.bin` artifact is verified and ready for deployment to `server` or `servant` hosts:

```bash
# Launch server with verified binary configuration
./server test_node.bin
```