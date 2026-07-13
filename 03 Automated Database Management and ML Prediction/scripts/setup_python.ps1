#!/usr/bin/env pwsh
$ErrorActionPreference = "Stop"
$root = $PSScriptRoot | Split-Path -Parent

# Use Python 3.13 explicitly; Python 3.14 pre-release builds have caused
# scipy/sklearn import crashes in this environment.
& py -3.13 -m venv "$root\.venv"
& "$root\.venv\Scripts\Activate.ps1"
python -m pip install --upgrade pip
pip install -r "$root\src\python\requirements.txt"
Write-Host "Python environment ready"
