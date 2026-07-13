#!/usr/bin/env pwsh
$ErrorActionPreference = "Stop"
$root = $PSScriptRoot | Split-Path -Parent
$venv = Join-Path $root ".venv\Scripts\Activate.ps1"

if (-not (Test-Path $venv)) {
    throw "Python virtual environment not found. Run setup_python.ps1 first."
}

& $venv
$env:PYTHONPATH = "$root\src\python"
python "$root\src\python\data\fetch_yahoo.py" --output-dir "$root\data\raw"
