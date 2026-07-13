#!/usr/bin/env pwsh
$ErrorActionPreference = "Stop"
$root = $PSScriptRoot | Split-Path -Parent
$junction = "C:\quant-pipeline"

# vcpkg ports for onnxruntime, libpq, and hwloc fail when the source path
# contains spaces. Build from a whitespace-free junction when available.
if ($root -match '\s' -and (Test-Path $junction)) {
    Write-Host "Detected spaces in project path; switching to junction $junction"
    $root = $junction
}

$buildDir = Join-Path $root "build"

if (-not $env:VCPKG_ROOT) {
    $candidates = @("C:\tools\vcpkg", "C:\Program Files\Microsoft Visual Studio\18\Community\VC\vcpkg", "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\vcpkg")
    foreach ($c in $candidates) {
        if (Test-Path (Join-Path $c "scripts\buildsystems\vcpkg.cmake")) {
            $env:VCPKG_ROOT = $c
            Write-Host "Auto-detected VCPKG_ROOT=$c"
            break
        }
    }
}
if (-not $env:VCPKG_ROOT) {
    throw "VCPKG_ROOT environment variable is not set and vcpkg could not be auto-detected. Set it to your vcpkg installation root."
}

& "$PSScriptRoot\setup_external_deps.ps1"

New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
Push-Location $buildDir
try {
    cmake -S $root -B . -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
    if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed" }
    cmake --build . --config Release --parallel
    if ($LASTEXITCODE -ne 0) { throw "CMake build failed" }

    # Copy external runtime DLLs next to executables so the correct versions are
    # loaded instead of any older DLL that may appear earlier in PATH.
    $ortDir = Join-Path $root "external\onnxruntime-1.23.2\lib"
    $pgBin = "C:\Program Files\PostgreSQL\16\bin"
    $releaseDirs = @(
        (Join-Path $buildDir "src\cpp\Release"),
        (Join-Path $buildDir "tests\Release")
    )
    foreach ($dir in $releaseDirs) {
        if (Test-Path $dir) {
            Copy-Item -Path (Join-Path $ortDir "onnxruntime.dll") -Destination $dir -Force -ErrorAction SilentlyContinue
            Copy-Item -Path (Join-Path $ortDir "onnxruntime_providers_shared.dll") -Destination $dir -Force -ErrorAction SilentlyContinue
            Copy-Item -Path (Join-Path $pgBin "libpq.dll") -Destination $dir -Force -ErrorAction SilentlyContinue
        }
    }

    Write-Host "C++ build complete"
} finally {
    Pop-Location
}
