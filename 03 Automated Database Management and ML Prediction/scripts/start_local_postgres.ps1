#!/usr/bin/env pwsh
$ErrorActionPreference = "Stop"
$root = $PSScriptRoot | Split-Path -Parent
$pgBase = "C:\Program Files\PostgreSQL\16"
$pgData = "$pgBase\data"
$logDir = "$root\logs"

New-Item -ItemType Directory -Force -Path $logDir | Out-Null

$proc = Get-Process -Name "postgres" -ErrorAction SilentlyContinue
if ($proc) {
    Write-Host "PostgreSQL is already running (PID $($proc.Id))"
    return
}

if (-not (Test-Path "$pgBase\bin\postgres.exe")) {
    throw "PostgreSQL 16 not found at $pgBase. Install PostgreSQL 16 or update `$pgBase."
}

# Remove stale timescaledb preload if present so the server can start without the extension.
$conf = "$pgData\postgresql.conf"
if (Test-Path $conf) {
    (Get-Content $conf) | ForEach-Object { $_ -replace "shared_preload_libraries = 'timescaledb'", "shared_preload_libraries = ''" } | Set-Content $conf
}

# Ensure local connections can use trust auth for unattended operation.
$hba = "$pgData\pg_hba.conf"
if (Test-Path $hba) {
    (Get-Content $hba) | ForEach-Object { $_ -replace 'scram-sha-256', 'trust' } | Set-Content $hba
}

Write-Host "Starting PostgreSQL from $pgData ..."
Start-Process -FilePath "$pgBase\bin\postgres.exe" `
    -ArgumentList "-D `"$pgData`"" `
    -WindowStyle Hidden `
    -RedirectStandardOutput "$logDir\postgres_stdout.log" `
    -RedirectStandardError "$logDir\postgres_stderr.log" `
    -PassThru | Out-Null

$attempts = 0
while ($attempts -lt 30) {
    Start-Sleep -Milliseconds 500
    try {
        $ver = psql -w -U postgres -c "SELECT version();" 2>$null
        if ($ver) {
            Write-Host "PostgreSQL started successfully"
            return
        }
    } catch {}
    $attempts++
}

throw "PostgreSQL failed to start. Check $logDir\postgres_stderr.log"
