#!/usr/bin/env pwsh
$ErrorActionPreference = "Stop"
$root = $PSScriptRoot | Split-Path -Parent

& "$PSScriptRoot\start_infra.ps1"
Start-Sleep -Seconds 5
& "$PSScriptRoot\generate_sample_data.ps1"
& "$PSScriptRoot\build_cpp.ps1"
& "$PSScriptRoot\run_etl.ps1"
& "$PSScriptRoot\run_features.ps1"
& "$PSScriptRoot\setup_python.ps1"
& "$PSScriptRoot\train_model.ps1"
& "$PSScriptRoot\run_inference.ps1"

Write-Host "Full pipeline completed"
