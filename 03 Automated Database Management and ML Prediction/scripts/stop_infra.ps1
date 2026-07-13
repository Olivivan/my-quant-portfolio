#!/usr/bin/env pwsh
$ErrorActionPreference = "Stop"
$root = $PSScriptRoot | Split-Path -Parent
$dockerDir = Join-Path $root "docker"

Push-Location $dockerDir
try {
    docker compose down
    Write-Host "Infrastructure stopped"
} finally {
    Pop-Location
}
