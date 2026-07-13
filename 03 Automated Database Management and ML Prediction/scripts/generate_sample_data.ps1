#!/usr/bin/env pwsh
$ErrorActionPreference = "Stop"
$root = $PSScriptRoot | Split-Path -Parent
$rawDir = Join-Path $root "data\raw"
$cleanDir = Join-Path $root "data\clean"
$parquetDir = Join-Path $root "data\parquet"
$modelDir = Join-Path $root "models"
$logDir = Join-Path $root "logs"

New-Item -ItemType Directory -Force -Path $rawDir | Out-Null
New-Item -ItemType Directory -Force -Path $cleanDir | Out-Null
New-Item -ItemType Directory -Force -Path $parquetDir | Out-Null
New-Item -ItemType Directory -Force -Path $modelDir | Out-Null
New-Item -ItemType Directory -Force -Path $logDir | Out-Null

$symbols = @("AAPL", "MSFT", "GOOGL", "TSLA")
$rng = New-Object System.Random

foreach ($sym in $symbols) {
    $path = Join-Path $rawDir "$sym.csv"
    $lines = New-Object System.Collections.Generic.List[System.String]
    $lines.Add("time,symbol,price,size,side,source")

    $start = Get-Date -Year 2024 -Month 1 -Day 1 -Hour 9 -Minute 30 -Second 0
    $price = 100.0 + $rng.NextDouble() * 100.0

    for ($i = 0; $i -lt 10000; $i++) {
        $dt = $start.AddSeconds($i)
        $ts = $dt.ToString("yyyy-MM-ddTHH:mm:ss.fff")
        $price = $price * (1.0 + ($rng.NextDouble() - 0.5) * 0.002)
        $size = [math]::Round($rng.NextDouble() * 1000 + 100, 2)
        $side = if ($rng.NextDouble() -gt 0.5) { 1 } else { -1 }
        $lines.Add("$ts,$sym,$($price.ToString('F4')),$size,$side,synthetic")
    }

    $lines | Out-File -FilePath $path -Encoding utf8
    Write-Host "Generated $path"
}
