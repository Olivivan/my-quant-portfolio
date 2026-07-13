#!/usr/bin/env pwsh
$ErrorActionPreference = "Stop"
$root = $PSScriptRoot | Split-Path -Parent

# 1. Start infrastructure and fetch full daily Yahoo Finance history for the top ten.
& "$PSScriptRoot\start_infra.ps1"
Start-Sleep -Seconds 3
& "$PSScriptRoot\fetch_yahoo_data.ps1"

# 2. ETL + features for every ticker.
#    This is done up-front so the model can be trained on all available data.
& "$PSScriptRoot\run_etl.ps1"
& "$PSScriptRoot\run_features.ps1"

# 3. Train the shared model once.
& "$PSScriptRoot\train_model.ps1"

# 4. Predictions are made one ticker at a time.
$symbols = @("AAPL", "MSFT", "NVDA", "AMZN", "GOOGL", "META", "TSLA", "AVGO", "BRK-B", "JPM")
foreach ($sym in $symbols) {
    Write-Host "`n=== ETL + features + inference for $sym ===" -ForegroundColor Cyan
    & "$PSScriptRoot\run_etl.ps1" -Symbol $sym
    & "$PSScriptRoot\run_features.ps1" -Symbol $sym
    & "$PSScriptRoot\run_inference.ps1" -Symbol $sym
}

Write-Host "`nYahoo Finance pipeline completed" -ForegroundColor Green
