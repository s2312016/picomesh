[CmdletBinding()]
param(
    [ValidateSet("pico", "pico2")]
    [string]$Board = "pico",

    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")),

    [string]$PicoSdkPath = $env:PICO_SDK_PATH,

    [string]$ToolchainRoot = $env:PICO_TOOLCHAIN_PATH,

    [string]$BuildDirectory,

    [switch]$Clean,

    [switch]$ConfigureOnly
)

$ErrorActionPreference = "Stop"

function Require-Command {
    param([Parameter(Mandatory = $true)][string]$Name)

    $command = Get-Command $Name -ErrorAction SilentlyContinue
    if (-not $command) {
        throw "Required command '$Name' was not found in PATH."
    }
    Write-Host "Found $Name at $($command.Source)"
    return $command
}

function Test-HostCompiler {
    foreach ($name in @("cl", "clang", "gcc")) {
        if (Get-Command $name -ErrorAction SilentlyContinue) {
            return $name
        }
    }
    return $null
}

$RepoRoot = (Resolve-Path $RepoRoot).Path
$example = Join-Path $RepoRoot "examples\PicoSdkI2cNode"
if (-not (Test-Path (Join-Path $example "CMakeLists.txt"))) {
    throw "PicoSdkI2cNode was not found under '$RepoRoot'. Run this script from a PicoMesh checkout or pass -RepoRoot."
}

if ([string]::IsNullOrWhiteSpace($PicoSdkPath)) {
    throw "PICO_SDK_PATH is not set. Pass -PicoSdkPath or set the environment variable."
}
$PicoSdkPath = (Resolve-Path $PicoSdkPath).Path
if (-not (Test-Path (Join-Path $PicoSdkPath "external\pico_sdk_import.cmake"))) {
    throw "'$PicoSdkPath' is not a valid Pico SDK checkout. external\pico_sdk_import.cmake is missing."
}
$env:PICO_SDK_PATH = $PicoSdkPath
Write-Host "PICO_SDK_PATH=$PicoSdkPath"

if (-not [string]::IsNullOrWhiteSpace($ToolchainRoot)) {
    $ToolchainRoot = (Resolve-Path $ToolchainRoot).Path
    $armCompiler = Join-Path $ToolchainRoot "bin\arm-none-eabi-gcc.exe"
    if (-not (Test-Path $armCompiler)) {
        throw "ARM compiler was not found at '$armCompiler'."
    }
    $env:PICO_TOOLCHAIN_PATH = $ToolchainRoot
    $toolchainBin = Join-Path $ToolchainRoot "bin"
    if (($env:Path -split ";") -notcontains $toolchainBin) {
        $env:Path = "$toolchainBin;$env:Path"
    }
    Write-Host "PICO_TOOLCHAIN_PATH=$ToolchainRoot"
}

Require-Command "cmake" | Out-Null
Require-Command "ninja" | Out-Null
Require-Command "arm-none-eabi-gcc" | Out-Null

$picotool = Get-Command "picotool" -ErrorAction SilentlyContinue
$hostCompiler = Test-HostCompiler
if (-not $picotool -and -not $hostCompiler) {
    throw @"
No installed picotool or native Windows C/C++ compiler was found.
Pico SDK may need to build picotool during configuration. Open 'Developer PowerShell for VS 2022'
or load Launch-VsDevShell.ps1, then run this script again.
"@
}
if ($picotool) {
    Write-Host "Found installed picotool at $($picotool.Source)"
} else {
    Write-Host "Native compiler available for picotool: $hostCompiler"
}

if ([string]::IsNullOrWhiteSpace($BuildDirectory)) {
    $BuildDirectory = Join-Path $RepoRoot "build-$Board"
} elseif (-not [System.IO.Path]::IsPathRooted($BuildDirectory)) {
    $BuildDirectory = Join-Path $RepoRoot $BuildDirectory
}

if ($Clean -and (Test-Path $BuildDirectory)) {
    Write-Host "Removing $BuildDirectory"
    Remove-Item -Recurse -Force $BuildDirectory
}

Write-Host "Configuring $Board in $BuildDirectory"
& cmake -S $example -B $BuildDirectory -G Ninja "-DPICO_BOARD=$Board"
if ($LASTEXITCODE -ne 0) {
    throw "CMake configuration failed with exit code $LASTEXITCODE."
}

if ($ConfigureOnly) {
    Write-Host "Configuration completed."
    exit 0
}

Write-Host "Building $Board firmware"
& cmake --build $BuildDirectory --parallel
if ($LASTEXITCODE -ne 0) {
    throw "Build failed with exit code $LASTEXITCODE."
}

$uf2Files = Get-ChildItem $BuildDirectory -Recurse -Filter "*.uf2"
if (-not $uf2Files) {
    throw "Build completed but no UF2 file was found under '$BuildDirectory'."
}

Write-Host "Generated firmware:"
$uf2Files | ForEach-Object { Write-Host "  $($_.FullName)" }
