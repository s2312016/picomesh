# Building Pico firmware on Windows

This guide builds the standalone `PicoSdkI2cNode` example with CMake, Ninja, the GNU Arm Embedded Toolchain, and Pico SDK 2.3.0.

## Recommended directory layout

Keep the SDK and project in separate directories:

```text
C:\Users\you\Documents\picomesh-sdk\pico-sdk
C:\Users\you\Documents\picomesh-src
```

The directory passed to `PICO_SDK_PATH` must contain `external\pico_sdk_import.cmake`. The PicoMesh checkout must contain `examples\PicoSdkI2cNode\CMakeLists.txt`.

## Required tools

Confirm these commands in PowerShell:

```powershell
cmake --version
ninja --version
arm-none-eabi-gcc --version
git --version
```

Pico SDK may build `picotool` from source while configuring. On Windows this also requires either an installed compatible `picotool` or a native Windows C/C++ compiler. Visual Studio Build Tools with the **Desktop development with C++** workload is one supported option. Run from **Developer PowerShell for VS 2022** so `cl.exe` is available.

```powershell
cl
```

The ARM compiler builds firmware for the microcontroller. `cl.exe`, native Clang, or native GCC builds host-side tools such as `picotool`; they are separate requirements.

## Clone Pico SDK

```powershell
cd C:\Users\you\Documents\picomesh-sdk
git clone --branch 2.3.0 --depth 1 https://github.com/raspberrypi/pico-sdk.git
git -C pico-sdk submodule update --init --recursive
```

A detached HEAD message is normal when cloning a release tag.

## Set environment variables

Adapt the paths to the installed toolchain and SDK:

```powershell
$toolchainRoot = "C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\14.2 rel1"
$env:Path = "$toolchainRoot\bin;$env:Path"
$env:PICO_TOOLCHAIN_PATH = $toolchainRoot
$env:PICO_SDK_PATH = "C:\Users\you\Documents\picomesh-sdk\pico-sdk"
```

Validate the SDK and compiler:

```powershell
Test-Path "$env:PICO_SDK_PATH\external\pico_sdk_import.cmake"
Test-Path "$env:PICO_TOOLCHAIN_PATH\bin\arm-none-eabi-gcc.exe"
arm-none-eabi-gcc --version
```

Both `Test-Path` commands should return `True`.

## Use the helper script

From a PicoMesh checkout:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\scripts\build_pico_windows.ps1 `
  -Board pico `
  -PicoSdkPath "C:\Users\you\Documents\picomesh-sdk\pico-sdk" `
  -ToolchainRoot "C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\14.2 rel1" `
  -Clean
```

For Pico 2, use `-Board pico2`.

`-Scope Process` changes the execution policy only for the current PowerShell process. The helper validates the repository, SDK, ARM compiler, Ninja, and host-side `picotool` build prerequisites before configuring.

## Manual build

```powershell
cd C:\Users\you\Documents\picomesh-src
Remove-Item -Recurse -Force .\build-pico -ErrorAction SilentlyContinue

cmake `
  -S .\examples\PicoSdkI2cNode `
  -B .\build-pico `
  -G Ninja `
  -DPICO_BOARD=pico

cmake --build .\build-pico --parallel
Get-ChildItem .\build-pico -Recurse -Filter *.uf2
```

Copy the generated UF2 to the board while holding BOOTSEL during USB connection.

## Common failures

### `examples/PicoSdkI2cNode does not exist`

CMake is being run from the SDK directory or another parent directory rather than the PicoMesh checkout.

```powershell
Test-Path .\examples\PicoSdkI2cNode\CMakeLists.txt
```

Run the build only when this returns `True`, or pass an absolute source path.

### `Compiler 'arm-none-eabi-gcc' not found`

The ARM toolchain is absent from `PATH`, or `PICO_TOOLCHAIN_PATH` points to the wrong directory. It must point to the directory containing the `bin` folder, not directly to `bin` and not to the Pico SDK.

```powershell
$env:Path = "$env:PICO_TOOLCHAIN_PATH\bin;$env:Path"
```

### `NMake Makefiles` or `CMAKE_MAKE_PROGRAM is not set`

CMake selected NMake but the Visual Studio development environment was not loaded. Install Ninja and select it explicitly:

```powershell
cmake -S .\examples\PicoSdkI2cNode -B .\build-pico -G Ninja -DPICO_BOARD=pico
```

Delete the old build directory before changing generators.

### `No CMAKE_C_COMPILER could be found` while building `picotool`

The ARM compiler was found, but no native Windows compiler is available for the host-side `picotool` executable. Open Developer PowerShell and confirm:

```powershell
cl
```

When the Visual Studio shell script is blocked by PowerShell policy, enable it only for the current process:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
& "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64
```

### No UF2 appears

CMake configuration alone does not produce firmware. Run the build command and inspect its exit code before searching for UF2 files.

```powershell
cmake --build .\build-pico --parallel
```

## Evidence boundary

A successful build proves that the example compiles for the selected board and toolchain. It does not prove electrical safety, I2C interoperability, timing margin, or runtime behavior on physical boards. Record those separately with the hardware validation procedure.
