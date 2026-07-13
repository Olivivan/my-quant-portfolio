#!/usr/bin/env pwsh
$ErrorActionPreference = "Stop"
$root = $PSScriptRoot | Split-Path -Parent
$ext = Join-Path $root "external"
$pgBase = "C:\Program Files\PostgreSQL\16"

New-Item -ItemType Directory -Force -Path $ext | Out-Null

# -----------------------------------------------------------------------------
# 1. ONNX Runtime prebuilt binaries (avoid long vcpkg source build and spaces-in-path bug)
# -----------------------------------------------------------------------------
$ortVersion = "1.23.2"
$ortZip = "onnxruntime-win-x64-$ortVersion.zip"
$ortUrl = "https://github.com/microsoft/onnxruntime/releases/download/v$ortVersion/$ortZip"
$ortDir = Join-Path $ext "onnxruntime-$ortVersion"

if (-not (Test-Path $ortDir)) {
    Write-Host "Downloading ONNX Runtime $ortVersion ..."
    Invoke-WebRequest -Uri $ortUrl -OutFile (Join-Path $ext $ortZip) -UseBasicParsing
    Expand-Archive -Path (Join-Path $ext $ortZip) -DestinationPath $ext -Force
    Remove-Item (Join-Path $ext $ortZip)
    # The archive extracts to a folder named onnxruntime-win-x64-<version>
    Rename-Item -Path (Join-Path $ext "onnxruntime-win-x64-$ortVersion") -NewName $ortDir -Force
}

# -----------------------------------------------------------------------------
# 2. Build libpqxx against the locally installed PostgreSQL 16 libpq
# -----------------------------------------------------------------------------
$pqxxVersion = "7.9.2"
$pqxxDir = Join-Path $ext "libpqxx-$pqxxVersion"
$pqxxBuild = Join-Path $pqxxDir "build"
$pqxxInstall = Join-Path $ext "libpqxx-install"

if (-not (Test-Path $pqxxInstall)) {
    if (-not (Test-Path $pqxxDir)) {
        Write-Host "Cloning libpqxx $pqxxVersion ..."
        git clone --depth 1 --branch $pqxxVersion https://github.com/jtv/libpqxx.git $pqxxDir
    }

    Remove-Item -Recurse -Force -ErrorAction SilentlyContinue $pqxxBuild
    New-Item -ItemType Directory -Force -Path $pqxxBuild | Out-Null

    Write-Host "Configuring libpqxx against system PostgreSQL ..."
    $cmakeArgs = @(
        "-S", $pqxxDir
        "-B", $pqxxBuild
        "-DCMAKE_BUILD_TYPE=Release"
        "-DCMAKE_INSTALL_PREFIX=$pqxxInstall"
        "-DPostgreSQL_ROOT=$pgBase"
        "-DPostgreSQL_INCLUDE_DIR=$pgBase\include"
        "-DPostgreSQL_LIBRARY=$pgBase\lib\libpq.lib"
        "-DSKIP_BUILD_TEST=ON"
        "-DBUILD_SHARED_LIBS=OFF"
    )
    & cmake @cmakeArgs
    if ($LASTEXITCODE -ne 0) { throw "libpqxx CMake configuration failed" }

    Write-Host "Building libpqxx ..."
    cmake --build $pqxxBuild --config Release --parallel
    if ($LASTEXITCODE -ne 0) { throw "libpqxx build failed" }

    Write-Host "Installing libpqxx ..."
    cmake --install $pqxxBuild --config Release
    if ($LASTEXITCODE -ne 0) { throw "libpqxx install failed" }
}

Write-Host "External dependencies ready:"
Write-Host "  ONNX Runtime: $ortDir"
Write-Host "  libpqxx:      $pqxxInstall"
