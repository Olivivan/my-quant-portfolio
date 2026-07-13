#!/usr/bin/env pwsh
$ErrorActionPreference = "Stop"
$root = $PSScriptRoot | Split-Path -Parent

python -m venv "$root\.venv"
& "$root\.venv\Scripts\Activate.ps1"
python -m pip install --upgrade pip
pip install -r "$root\src\python\requirements.txt"
Write-Host "Python environment ready"
