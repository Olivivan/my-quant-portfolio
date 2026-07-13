# Installation Guide

## Prerequisites

- Windows 10/11
- PowerShell 5.1 or PowerShell 7+
- [Git](https://git-scm.com/download/win)
- [CMake](https://cmake.org/download/) ≥ 3.21
- [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/) with "Desktop development with C++" workload
- [vcpkg](https://github.com/microsoft/vcpkg) (optional but recommended for C++ deps)
- [Docker Desktop](https://www.docker.com/products/docker-desktop/) with WSL2 backend
- [Python](https://www.python.org/downloads/) ≥ 3.10, **3.13 recommended** (Python 3.14 pre-release builds are not supported due to scipy/sklearn incompatibilities)
- [Node.js](https://nodejs.org/) (only if using ONNX Runtime web features, not required here)

## Install C++ Dependencies via vcpkg

```powershell
# In a PowerShell admin/regular prompt:
git clone https://github.com/microsoft/vcpkg.git C:\tools\vcpkg
C:\tools\vcpkg\bootstrap-vcpkg.bat
C:\tools\vcpkg\vcpkg install arrow parquet libpqxx fmt spdlog tbb onnxruntime xsimd --triplet x64-windows
```

Set the environment variable:

```powershell
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\tools\vcpkg", "User")
```

## Install Python Dependencies

```powershell
python -m venv .venv
.\venv\Scripts\Activate.ps1
pip install -r src/python/requirements.txt
```

## Start Infrastructure

```powershell
.\scripts\start_infra.ps1
```

If Docker is available and running, this starts:

- TimescaleDB on `localhost:5432`
- Redis on `localhost:6379`

If Docker is unavailable, the script falls back to a local PostgreSQL 16
instance (expected at `C:\Program Files\PostgreSQL\16`), initializes the
`quantdb` database, and creates the `quant` user. TimescaleDB hypertables are
used automatically when the extension is installed; otherwise the schema falls
back to plain PostgreSQL tables.

## Verify

```powershell
python -c "import lightgbm, xgboost, sklearn, onnx, pandas; print('OK')"
.\scripts\build_cpp.ps1
```
