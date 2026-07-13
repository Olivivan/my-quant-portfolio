#!/usr/bin/env pwsh
$ErrorActionPreference = "Stop"
$root = $PSScriptRoot | Split-Path -Parent
$schema = Join-Path $root "docker\init\01_schema.sql"

$exists = psql -w -U postgres -tc "SELECT datname FROM pg_database WHERE datname = 'quantdb';" | Select-String "quantdb"
if (-not $exists) {
    Write-Host "Creating database quantdb..."
    psql -w -U postgres -c "CREATE DATABASE quantdb;"
}

Write-Host "Applying schema (TimescaleDB when available, plain PostgreSQL fallback)..."
psql -w -U postgres -d quantdb -f $schema

$quantExists = psql -w -U postgres -tc "SELECT rolname FROM pg_roles WHERE rolname = 'quant';" | Select-String "quant"
if (-not $quantExists) {
    Write-Host "Creating quant user..."
    psql -w -U postgres -c "CREATE USER quant WITH PASSWORD 'quantpass' CREATEDB;"
}

psql -w -U postgres -d quantdb -c "GRANT ALL PRIVILEGES ON DATABASE quantdb TO quant; GRANT USAGE, CREATE ON SCHEMA public TO quant; GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO quant; ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON TABLES TO quant; GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO quant;"
Write-Host "Database initialized"
