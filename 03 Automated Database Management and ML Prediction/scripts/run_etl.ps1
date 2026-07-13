#!/usr/bin/env pwsh
param([string]$Symbol = "")
$ErrorActionPreference = "Stop"
$root = $PSScriptRoot | Split-Path -Parent
$junction = "C:\quant-pipeline"

if ($root -match '\s' -and (Test-Path $junction)) {
    $root = $junction
}

$exe = Join-Path $root "build\src\cpp\Release\quant_etl.exe"

if (-not (Test-Path $exe)) {
    throw "quant_etl.exe not found. Run build_cpp.ps1 first."
}

& $exe "$root\config\pipeline.json" $Symbol
