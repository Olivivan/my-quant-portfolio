#!/usr/bin/env pwsh
$ErrorActionPreference = "Stop"
$root = $PSScriptRoot | Split-Path -Parent
$dockerDir = Join-Path $root "docker"

function Test-DockerRunning {
    if (-not (Get-Command docker -ErrorAction SilentlyContinue)) { return $false }
    try {
        $info = docker info 2>$null
        return ($LASTEXITCODE -eq 0)
    } catch { return $false }
}

if (Test-DockerRunning) {
    Push-Location $dockerDir
    try {
        if (-not (Test-Path .env)) {
            Copy-Item .env.example .env
            Write-Host "Created .env from .env.example"
        }
        docker compose up -d
        Write-Host "Infrastructure started: TimescaleDB + Redis (Docker)"
    } finally {
        Pop-Location
    }
} else {
    Write-Host "Docker not available; falling back to local PostgreSQL."
    & "$PSScriptRoot\start_local_postgres.ps1"
    & "$PSScriptRoot\init_db.ps1"
}
